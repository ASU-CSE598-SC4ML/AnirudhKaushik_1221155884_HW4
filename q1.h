//
// Created by anirudh on 24/04/21.
//

#ifndef ABY_Q1_H
#define ABY_Q1_H


int32_t logistic_regression(e_role role, const std::string& address, uint16_t port, seclvl seclvl,
                                      uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg, e_sharing sharing);

/**
 \param		s_alice		shared object of alice.
 \param		s_bob 		shared object of bob.
 \param		bc	 		boolean circuit object.
 \brief		This function is used to build and solve the millionaire's problem.
 */
share* BuildWeightUpdateCircuit(share *x, share *y, share *w,
                                ArithmeticCircuit *ac,std::vector<Sharing*>& sharings,uint32_t alpha,share *const1_s,share *const2_s);

int32_t test_logistic_regression(e_role role, const std::string& address, uint16_t port, seclvl seclvl,
                                      uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg, e_sharing sharing);


#endif //ABY_Q1_H
