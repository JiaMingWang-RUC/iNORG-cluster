#pragma once

/*
coded by Rong-Qiang He (rqhe@ruc.edu.cn, RUC, China) date 2021-02-19
code developed and maintained by (jmw@ruc.edu.cn, RUC, China) date 2023
*/

#include "specs.h"
#include "prmtr.h"
#include "green.h"
#include "hyberr.h"

class Bath {
public:
	const MyMpi& mm;
	const Prmtr& p;			// parameters
	Int nb;					// number of bath sites
	ImGreen hb;				// hybridization function
	UURandomSFMT uur;
	VecReal ose;			// on-site energies for bath sites
	VecReal hop;			// hopping coefficients between impurity site and bath sites

	VecReal osea;
	VecReal hopb;
	VecReal oseA;
	VecReal hopB;

	VEC<VecReal> vec_ose, vec_hop;
	MatReal info;		// print out the NAV5(nmin, err, err_crv, err_reg, /*err_bsr,*/ a_norm)
private:
	void regularize_ose_hop();
	void regularize_osea_hopb();
	void init_ose_hop() {
		uur(ose);
		ose -= 0.5;
		ose *= 4.;
		uur(hop);
		hop -= 0.5;
		hop *= 4.;
		regularize_ose_hop();
	}
	void init_osea_hopb() {
		uur(osea);
		osea -= 0.5;
		osea *= 4.;
		uur(hopb);
		hopb -= 0.5;
		hopb *= 4.;

		regularize_osea_hopb();
	}
	std::tuple<Real, VecReal, Int> bath_fit_contest(const VecReal& a0);
	std::tuple<Real, VecReal, Int> bath_fit_bsr(const VecReal& a0, const Int& orb_i);

	VecReal next_initial_fitting_parameters(const VecReal& a0, const Int& ntry_fine, Int& itry);
	void perturb(VecReal& a, Real amplitude, Real exponent) {
		for_Int(i, 0, a.size()) {
			Real ra = uur() - 0.5;
			Real re = uur() - 0.5;
			a[i] = a[i] * (1. + amplitude * ra) + SIGN(std::pow(10., (8 * ABS(re) - exponent)), a[i]);
		}
	}
public:
	Bath(const MyMpi& mm_i, const Prmtr& prmtr_i);
	void bath_fit(const ImGreen& hb_i, Int iter);
	void bath_fit(const ImGreen& hb_i, VecInt or_deg);


	void bath_TRfit(const ImGreen& hb_i, Int iter);//! NOT finished.
	MatReal find_hop() const;

	void write_ose_hop(Int iter_cnt = -1, const Str& bath_name = empty_str) const;
	void read_ose_hop();
};