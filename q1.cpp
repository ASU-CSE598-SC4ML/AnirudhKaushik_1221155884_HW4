#include "millionaire_prob.h"
#include "q1.h"
#include "../../../abycore/circuit/booleancircuits.h"
#include "../../../abycore/circuit/arithmeticcircuits.h"
#include "../../../abycore/sharing/sharing.h"

int32_t logistic_regression(e_role role, const std::string& address, uint16_t port, seclvl seclvl,
                                      uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg, e_sharing sharing) {

    ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads,
                                   mt_alg);

    std::vector<Sharing*>& sharings = party->GetSharings();

    Circuit* circ = sharings[sharing]->GetCircuitBuildRoutine();

    share *x, *y, *w, *s_out, *x2,*y2,*w2, *const1_s,*const2_s;

    uint32_t server2_x[5], server2_y[5],server1_x[5], server1_y[5], output, alpha, weight,const1,const2;
    srand(time(NULL));


    alpha = rand();
    weight = rand();
    const1 = -1/2;
    const2 = 1/2;

    /** Each server has 5 pairs of (x,y) values **/

    for(int i=0;i<5;i++)
    {
        server1_x[i] = rand();
        server1_y[i] = rand();

        server2_x[i] = rand();
        server2_y[i] = rand();
    }

    int epoch = 10;

    /** Train for 10 epochs **/
    while(epoch--) {
        for (int i = 0; i < 5; i++) {
            if (role == SERVER) {
                x = circ->PutINGate(server2_x[i], bitlen, SERVER);
                y = circ->PutINGate(server2_y[i], bitlen, SERVER);
                w = circ->PutINGate(weight, bitlen, SERVER);
                const1_s = circ->PutINGate(const1, bitlen, SERVER);
                const2_s = circ->PutINGate(const2, bitlen, SERVER);
            } else { //role == CLIENT
                x = circ->PutINGate(server1_x[i], bitlen, CLIENT);
                y = circ->PutINGate(server1_y[i], bitlen, CLIENT);
                w = circ->PutINGate(weight, bitlen, CLIENT);
                const1_s = circ->PutINGate(const1, bitlen, CLIENT);
                const2_s = circ->PutINGate(const2, bitlen, CLIENT);
            }
        }

        s_out = BuildWeightUpdateCircuit(x, y, w,
                                         (ArithmeticCircuit*) circ,sharings,alpha,const1_s,const2_s);
        s_out = circ->PutOUTGate(s_out, ALL);

        party->ExecCircuit();

        /** cleartext of weight outputs **/
        output = s_out->get_clear_value<uint32_t>();

        std::cout << "\nUpdated weight:\t" << output;

        party->Reset();
    }

    delete party;
    return 0;
}


share* BuildWeightUpdateCircuit(share *x, share *y, share *w,
                                ArithmeticCircuit *ac,std::vector<Sharing*>& sharings,uint32_t alpha,share *const1_s,share *const2_s) {

    share* out, *t_a, *t_b, *z, *res_y, *check_sel1,
            *check_sel2, *res, *arithmetic_share_fx;

    /** Following code performs (x*w) */
    z = ac->PutMULGate(x, w);

    /** Convert Arithmetic Share to Yao Share **/
    share * yao_z ;
    Circuit * yc = sharings [S_YAO] -> GetCircuitBuildRoutine() ;
    yao_z = yc -> PutA2YGate (z) ;

    /** Piece wise activation function f(x) **/
    check_sel1 = yc->PutGTGate(reinterpret_cast<share *>(-1 / 2),yao_z);
    check_sel2 = yc->PutGTGate(reinterpret_cast<share *>(1 / 2),yao_z);

    /**Using Nested MUX to return value of f(x) based on selection bits check_sel1 and check_sel2**/
    res = yc->PutMUXGate(reinterpret_cast<share *>(0),
                         (yc->PutMUXGate(reinterpret_cast<share *>(yao_z+1/2),reinterpret_cast<share *>(1),check_sel2)),
                         check_sel1);

    /**Convert output of f(x) back to arithmetic share **/
    arithmetic_share_fx = ac->PutY2AGate(res, sharings[S_BOOL]->GetCircuitBuildRoutine());

    t_a = ac->PutSubGate(arithmetic_share_fx,y);
    t_a = ac->PutMULGate(t_a,reinterpret_cast<share *>(alpha));
    t_a = ac->PutMULGate(t_a,x);

    /** Output shares of updated weight **/
    out = ac->PutSubGate(w,t_a);

    return out;
}
