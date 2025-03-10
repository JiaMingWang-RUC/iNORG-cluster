/*
code developed and maintained by (jmw@ruc.edu.cn, RUC, China) date 2022 - 2024
*/

#include "state.h"

using namespace std;
StateStatistics::StateStatistics(const Int& h_i, const Int& comdiv_idx, const NocSpace& scsp) :
	space(scsp), div_idx(comdiv_idx), idx(h_i - scsp.idx_div[comdiv_idx]),
	occ_n(scsp.div[comdiv_idx]), cfg(cfgdetail()), filled_spinless(scsp.sit_mat.nrows())
	// cfgup(cfg.cf.truncate(0, scsp.ndivs)), cfgdw(cfg.cf.truncate(scsp.ndivs, scsp.ndivs + scsp.ndivs))
{
	//DBG("statistics BEGIN: " + NAV(idx));
	statistics();
}

vector<VecInt> StateStatistics::find_each_spiless_group_off_diagonal_term(const hopdata &hopspi, const Int sets_n)
{
	vector<VecInt> off_diagonal_term;
	{
		VecInt ODT_i(4, 0);// [0]\([1]):annihilation\(creation) orbit's position;[2]:Colum idx(i);[i][3]:sign.
		//off-diagonal, two-fermion operator terms for the Up spin C^+_i C_j
		for_Int(i, 0, hopspi.size()) {
			Idx idx_div_i(find_newdiv_idx(get<2>(hopspi[i])));
			Int div_pst_ann(get<0>(hopspi[i]) + sets_n * space.ndivs), div_pst_crt(get<1>(hopspi[i]) + sets_n * space.ndivs);

			for_Int(c, 0, cfg.div_orb_e[div_pst_ann].size()) {
				for_Int(cp, 0, cfg.div_orb_c[div_pst_crt].size()) {
					// for_Int(k, 0, sets_n){ ODT_i[0] += SUM(space.sit_mat[k]); ODT_i[1] += SUM(space.sit_mat[k]); }
					ODT_i[0] = (SUM_0toX(space.sit_mat, sets_n, get<0>(hopspi[i])) + cfg.div_orb_e[div_pst_ann][c]);
					ODT_i[1] = (SUM_0toX(space.sit_mat, sets_n, get<1>(hopspi[i])) + cfg.div_orb_c[div_pst_crt][cp]);
					VecOnb newcf(cfg.cf);
					#ifdef _ASSERTION_
						if (newcf[div_pst_ann].isuno(cfg.div_orb_e[div_pst_ann][c]))ERR("This position can't ann");
						if (newcf[div_pst_crt].isocc(cfg.div_orb_c[div_pst_crt][cp]))ERR("This position can't crt");
					#endif
					newcf[div_pst_ann] = newcf[div_pst_ann].ann(cfg.div_orb_e[div_pst_ann][c]);
					newcf[div_pst_crt] = newcf[div_pst_crt].crt(cfg.div_orb_c[div_pst_crt][cp]);
					
					ComDivs b(newcf, (get<2>(hopspi[i])), (space.sit_mat));
					ODT_i[2] = idx_div_i + b.idx;
					if (ODT_i[2] > space.dim) ERR(STR("Hmlt Off-Diag Elements IHTL > IHM ") + NAV2(ODT_i[2], space.dim));
					ODT_i[3] = cfg.sgn(ODT_i[0], ODT_i[1]);
					#ifdef _ASSERTION_
						if (ODT_i[0] == ODT_i[1]) ERR("A impossible thing happened:ODT_i[0] == ODT_i[1]" + NAV(ODT_i[0]));
					#endif
					off_diagonal_term.push_back(ODT_i);
				}
			}
		}
	}
	return off_diagonal_term;
}

VEC<array<int, 6>> StateStatistics::find_off_diagonal_term_fourFermi(const furfrm &furfrm, const Int sets_i, const Int sets_j)
{
	VEC<array<int, 6>> off_diagonal_term;
	{
		// [0]~[3] i-j-k-l orbit's position; [4]:Colum idx(i);[5]:sign.
		//off-diagonal, four-fermion operator terms for C^+_i C^+_j C_k C_l
		array<int, 6> ODT_i = {0,0,0,0,0,0};
		
		for_Int(i, 0, furfrm.size()) {
			Idx idx_div_i(find_newdiv_idx(furfrm[i].second));
			Int div_pst_crt_i(furfrm[i].first[0] + sets_i * space.ndivs), div_pst_crt_j(furfrm[i].first[1] + sets_j * space.ndivs), 
				div_pst_ann_k(furfrm[i].first[2] + sets_j * space.ndivs), div_pst_ann_l(furfrm[i].first[3] + sets_i * space.ndivs);

			for_Int(cp_i, 0, cfg.div_orb_c[div_pst_crt_i].size()) {
			for_Int(cp_j, 0, cfg.div_orb_c[div_pst_crt_j].size()) {
			for_Int(c__k, 0, cfg.div_orb_e[div_pst_ann_k].size()) {
			for_Int(c__l, 0, cfg.div_orb_e[div_pst_ann_l].size()) {

				ODT_i[0] = (SUM_0toX(space.sit_mat, sets_i, furfrm[i].first[0]) + cfg.div_orb_c[div_pst_crt_i][cp_i]);
				ODT_i[1] = (SUM_0toX(space.sit_mat, sets_j, furfrm[i].first[1]) + cfg.div_orb_c[div_pst_crt_j][cp_j]);
				ODT_i[2] = (SUM_0toX(space.sit_mat, sets_j, furfrm[i].first[2]) + cfg.div_orb_e[div_pst_ann_k][c__k]);
				ODT_i[3] = (SUM_0toX(space.sit_mat, sets_i, furfrm[i].first[3]) + cfg.div_orb_e[div_pst_ann_l][c__l]);
				if(ODT_i[0] != ODT_i[1] && ODT_i[2] != ODT_i[3]) {
					VecOnb newcf(cfg.cf);
					
					newcf[div_pst_crt_i] = newcf[div_pst_crt_i].crt(cfg.div_orb_c[div_pst_crt_i][cp_i]);
					newcf[div_pst_crt_j] = newcf[div_pst_crt_j].crt(cfg.div_orb_c[div_pst_crt_j][cp_j]);
					newcf[div_pst_ann_k] = newcf[div_pst_ann_k].ann(cfg.div_orb_e[div_pst_ann_k][c__k]);
					newcf[div_pst_ann_l] = newcf[div_pst_ann_l].ann(cfg.div_orb_e[div_pst_ann_l][c__l]);
					
					ComDivs b(newcf, furfrm[i].second, space.sit_mat);
					ODT_i[4] = idx_div_i + b.idx;
					if (ODT_i[4] > space.dim) ERR(STR("Hmlt Off-Diag Elements IHTL > IHM ") + NAV2(ODT_i[2], space.dim));
					ODT_i[5] = cfg.sgn(ODT_i[0], ODT_i[3]);	// sign for C^+_i C_l
					ODT_i[5] *= cfg.sgn(ODT_i[1], ODT_i[2]);// sign for C^+_j C_k
					auto [min, max] = minmax(ODT_i[0], ODT_i[3]);
					if (ODT_i[1] == myclamp(ODT_i[1], min, max)) ODT_i[5] *= -1;
					if (ODT_i[2] == myclamp(ODT_i[2], min, max)) ODT_i[5] *= -1;
					off_diagonal_term.push_back(ODT_i);
				}
			}
			}
			}
			}
		}
	}
	return off_diagonal_term;
}

VEC<VecInt> StateStatistics::off_diagonal_soc_term(const VEC<MatInt> &hop_soc)
{
	VEC<VecInt> off_diagonal_term;
	{
		VecInt ODT_i(4, 0);// [0]\([1]):annihilation\(creation) orbit's position;[2]:Colum idx(i);[i][3]:sign.

		//off-diagonal, SOC operator terms
		for_Int(i, 0, hop_soc.size()) {
			Idx idx_div_i(find_newdiv_idx(hop_soc[i]));
			VecOnb newcf(cfg.cf);
			
			for_Int(orb, 0, occ_n.nrows()) {
				Int a = occ_n[orb][0] - hop_soc[i][orb][0];
				if(a = 1)			newcf[orb * occ_n.ncols()] = newcf[orb * occ_n.ncols()].ann(0);
				else if(a = -1)		newcf[orb * occ_n.ncols()] = newcf[orb * occ_n.ncols()].crt(0);
				else ERR("This position can't ann");
			}
			
			ComDivs b(newcf, hop_soc[i], space.sit_mat);
			ODT_i[2] = idx_div_i + b.idx;
			if (ODT_i[2] > space.dim) ERR(STR("Hmlt Off-Diag Elements IHTL > IHM ") + NAV2(ODT_i[2], space.dim));
			ODT_i[3] = cfg.sgn(ODT_i[0], ODT_i[1]);
			#ifdef _ASSERTION_
				if (ODT_i[0] == ODT_i[1]) ERR("A impossible thing happened:ODT_i[0] == ODT_i[1]" + NAV(ODT_i[0]));
			#endif
			off_diagonal_term.push_back(ODT_i);
		}
	}
	return off_diagonal_term;
}

// This  function is Deprecation!
/* vector<VecInt> StateStatistics::find_off_diagonal_term(const hopdata& hopup, const hopdata& hopdw)
{
	vector<VecInt> off_diagonal_term;
	// UP term.
	{
		VecInt ODT_i(4, 0);// [0]\([1]):annihilation\(creation) orbit's position;[2]:Colum idx(i);[i][3]:sign.
		//off-diagonal, two-fermion operator terms for the Up spin C^+_i C_j
		for_Int(i, 0, hopup.size()) {
			Idx idx_div_i(find_newdiv_idx(get<2>(hopup[i])));
			for_Int(c, 0, cfg.div_orb_e[get<0>(hopup[i])].size()) {
				for_Int(cp, 0, cfg.div_orb_c[get<1>(hopup[i])].size()) {
					ODT_i[0] = (SUM_0toX(sit_n, get<0>(hopup[i])) + cfg.div_orb_e[get<0>(hopup[i])][c]);
					ODT_i[1] = (SUM_0toX(sit_n, get<1>(hopup[i])) + cfg.div_orb_c[get<1>(hopup[i])][cp]);
					VecOnb newcf(cfg.cf);
#ifdef _ASSERTION_
					if (newcf[get<0>(hopup[i])].isuno(cfg.div_orb_e[get<0>(hopup[i])][c]))ERR("This position can't ann");
					if (newcf[get<1>(hopup[i])].isocc(cfg.div_orb_c[get<1>(hopup[i])][cp]))ERR("This position can't crt");
#endif
					newcf[get<0>(hopup[i])] = newcf[get<0>(hopup[i])].ann(cfg.div_orb_e[get<0>(hopup[i])][c]);
					newcf[get<1>(hopup[i])] = newcf[get<1>(hopup[i])].crt(cfg.div_orb_c[get<1>(hopup[i])][cp]);
#ifdef _ASSERTION_
					VecInt ne_b(occ_n);
					--ne_b[get<0>(hopup[i])];
					++ne_b[get<1>(hopup[i])];
					if (ne_b != get<2>(hopup[i])) ERR("NOT equal" + NAV4(ne_b, get<0>(hopup[i]), get<1>(hopup[i]), get<2>(hopup[i])));
#endif
					ComDivs b(newcf, get<2>(hopup[i]), sit_n);
					ODT_i[2] = idx_div_i + b.idx;
					if (ODT_i[2] > space.dim) ERR(STR("Hmlt Off-Diag Elements IHTL > IHM ") + NAV2(ODT_i[2], space.dim));
					ODT_i[3] = cfg.sgn(ODT_i[0], ODT_i[1]);
#ifdef _ASSERTION_
					if (ODT_i[0] == ODT_i[1]) ERR("A impossible thing happened:ODT_i[0] == ODT_i[1]" + NAV(ODT_i[0]));
#endif
					off_diagonal_term.push_back(ODT_i);
				}
			}
		}
	}

	// DOWN term.
	{
		VecInt ODT_i(4, 0);// [0]/([1]):annihilation/(creation) orbit's position;[2]:Colum idx(i);[i][3]:sign.
		//off-diagonal, two-fermion operator terms for the down spin C^+_i C_j
		for_Int(i, 0, hopdw.size()) {
			Idx idx_div_i(find_newdiv_idx(get<2>(hopdw[i])));
			for_Int(c, 0, cfg.div_orb_e[get<0>(hopdw[i])].size()) {
				for_Int(cp, 0, cfg.div_orb_c[get<1>(hopdw[i])].size()) {
					ODT_i[0] = (SUM_0toX(sit_n, get<0>(hopdw[i])) + cfg.div_orb_e[get<0>(hopdw[i])][c]);
					ODT_i[1] = (SUM_0toX(sit_n, get<1>(hopdw[i])) + cfg.div_orb_c[get<1>(hopdw[i])][cp]);
					VecOnb newcf(cfg.cf);
#ifdef _ASSERTION_
					if (newcf[get<0>(hopdw[i])].isuno(cfg.div_orb_e[get<0>(hopdw[i])][c]))ERR("This position can't ann");
					if (newcf[get<1>(hopdw[i])].isocc(cfg.div_orb_c[get<1>(hopdw[i])][cp]))ERR("This position can't crt");
#endif
					newcf[get<0>(hopdw[i])] = newcf[get<0>(hopdw[i])].ann(cfg.div_orb_e[get<0>(hopdw[i])][c]);
					newcf[get<1>(hopdw[i])] = newcf[get<1>(hopdw[i])].crt(cfg.div_orb_c[get<1>(hopdw[i])][cp]);
#ifdef _ASSERTION_
					VecInt ne_b(occ_n);
					--ne_b[get<0>(hopdw[i])];
					++ne_b[get<1>(hopdw[i])];
					if (ne_b != get<2>(hopdw[i])) ERR("NOT equal" + NAV4(ne_b, get<0>(hopdw[i]), get<1>(hopdw[i]), get<2>(hopdw[i])));
#endif
					ComDivs b(newcf, get<2>(hopdw[i]), sit_n);
					ODT_i[2] = idx_div_i + b.idx;
					if (ODT_i[2] > space.dim) ERR(STR("Hmlt Off-Diag Elements IHTL > IHM ") + NAV2(ODT_i[2], space.dim));
					ODT_i[3] = cfg.sgn(ODT_i[0], ODT_i[1]);
#ifdef _ASSERTION_
					if (ODT_i[0] == ODT_i[1]) ERR("A impossible thing happened:ODT_i[0] == ODT_i[1]" + NAV(ODT_i[0]));
#endif
					off_diagonal_term.push_back(ODT_i);
				}
			}
		}
	}
	return off_diagonal_term;
} */

Int StateStatistics::find_newdiv_idx(MatInt newdiv)
{

	return space.divs_to_idx.at(newdiv.vec().string());

	ERR("NOT found the div in the short cut space.");
}

StateStatistics::hopdata StateStatistics::divocchop_ingroup(const Int& ComDiv, Idx sets_n)
{
	hopdata hop;
	// "c": means annihilation, and "cp" mean creation. Which is act on all div.
	// For the spinless orbits.
	for_Int(c, 0, space.ndivs){				//Here "1" refer to div_idx, according to the "set_control()" function.
		for_Int(cp, 0, space.ndivs){		//Here "1" refer to div_idx, according to the "set_control()" function.
			MatInt occupy(space.div[ComDiv]);
			--occupy[sets_n][c];
			++occupy[sets_n][cp];
			// if (space.ifin_NocSpace(occupy)) {
			if (space.ifin_NocSpace(occupy, space.nppso)) {
				tuple<Int, Int, MatInt> tup1(make_tuple(c, cp, occupy));
				hop.push_back(tup1);
			}
		}
	}
	return hop;
}

StateStatistics::furfrm StateStatistics::divs_change_fourFermi(const Int& ComDiv, Idx sets_i, Idx sets_j)
{
	furfrm hop;
	// "c": means annihilation, and "cp" mean creation. Which is act on all div.
	// For the spinless orbits.
	for_Int(cp_i, 0, space.ndivs) {
	for_Int(cp_j, 0, space.ndivs) {
		for_Int(c_k, 0, space.ndivs) {
		for_Int(c_l, 0, space.ndivs) {
			MatInt occupy(space.div[ComDiv]);
			++occupy[sets_i][cp_i]; ++occupy[sets_j][cp_j]; 
			--occupy[sets_j][c_k]; --occupy[sets_i][c_l]; 
			if (space.ifin_NocSpace(occupy, space.nppso)) {
				pair<array<int, 4>, MatInt> tup1(make_pair(array<int, 4>{cp_i, cp_j, c_k, c_l}, occupy));
				hop.push_back(tup1);
			}
		}
		}
	}
	}
	return hop;
}

VEC<MatInt> StateStatistics::interation_soc_hop(const Int& ComDiv)
{
	VEC<MatInt> hop_soc;
	// For the spinless orbits.
	
	MatInt occupy = space.div[ComDiv];
	WRN(NAV(occupy));
	for_Int(i, 0, space.p.nband) {
		Int cnt(occupy[i*2][0] + occupy[i*2+1][0]);
		if(cnt == 0) {
			for_Int(j, 0, space.p.nband) if(i != j) {
				Int cnt_j(occupy[j*2][0] + occupy[j*2+1][0]);
				if(cnt_j == 2) {
					occupy[i*2][0] = occupy[i*2+1][0] = 1;
					occupy[j*2][0] = occupy[j*2+1][0] = 0;
					if (space.ifin_NocSpace(occupy, space.nppso)) hop_soc.push_back(occupy);
					WRN(NAV(occupy));
				}
			}
		}

		if(cnt == 2) {
			for_Int(j, 0, space.p.nband) if(i != j) {
				Int cnt_j(occupy[j*2][0] + occupy[j*2+1][0]);
				if(cnt_j == 0) {
					occupy[i*2][0] = occupy[i*2+1][0] = 0;
					occupy[j*2][0] = occupy[j*2+1][0] = 1;
					if (space.ifin_NocSpace(occupy, space.nppso)) hop_soc.push_back(occupy);
					WRN(NAV(occupy));
				}
			}
		}

		if(cnt == 1) {
			for_Int(j, 0, space.p.nband) if(i != j) {
				Int cnt_j(occupy[j*2][0] + occupy[j*2+1][0]);
				if(cnt == 1 && occupy[j*2][0] != occupy[i*2][0]) {
					SWAP(occupy[i*2][0], occupy[i*2+1][0]);
					SWAP(occupy[j*2][0], occupy[j*2+1][0]);
					if (space.ifin_NocSpace(occupy, space.nppso)) hop_soc.push_back(occupy);
					WRN(NAV(occupy));
				}
			}
		}
	}

	return hop_soc;
}

Mat<Char> StateStatistics::show_cfg() {
	const VecOnb& cf(cfg.cf);
	Str cfig;
	for_Int(i, 0, cf.size()){
		for_Int(j, 0, cf[i].get_ns()){
			cfig += cf[i][j] ? '1':'0';
		}
	}
	Vec<Char> ci(cfig.length());
	for_Int(i, 0, ci.size()) ci[i] = cfig[i];
	// if(space.mm) WRN(NAV(ci.mat(space.p.norbs,space.p.nI2B[0]+1)));
	return ci.mat(space.p.norbs,space.p.nI2B[0]+1);
}

/* version 1
array<UInt,6> StateStatistics::cfg2nums() {
	// UInt nums[occ_n.nrows()];
	array<UInt,6> nums;
	const VecOnb& cf(cfg.cf);
	for_Idx(i, 0, cf.size()){
		Str cfig;
		for_Int(j, 0, cf[i].get_ns()) cfig += cf[i][j];
		const UInt num {stoul(cfig, nullptr, 2)};
		nums[i] = num;
	}
	return move(nums);
}
*/

// version 2
VecBool StateStatistics::cfg2vecbool() {
	const VecOnb cf(cfg.cf);
	
	Str cfig;
	for_Int(i, 0, cf.size()){
		for_Int(j, 0, cf[i].get_ns()){
			cfig += cf[i][j] ? '1':'0';
		}
	}
	VecBool ci(cfig.length());
	for_Int(i, 0, ci.size()) ci[i] = cfig[i] == '1' ? true : false;

	return move(ci);
}

// version 2
VecBool StateStatistics::cfgex2vecbool(Int ex_pos) {
	
	MatOnb cf_temp(cfg.cf.mat(occ_n.nrows(), occ_n.ncols()));
	Idx pos(ABS(ex_pos) - 1);
	cf_temp[pos][0] = ex_pos > 0 ? cf_temp[pos][0].crt(0) : cf_temp[pos][0].ann(0);
	
	const VecOnb cf(cf_temp.vec());
	
	Str cfig;
	for_Int(i, 0, cf.size()){
		for_Int(j, 0, cf[i].get_ns()){
			cfig += cf[i][j] ? '1':'0';
		}
	}
	VecBool ci(cfig.length());
	for_Int(i, 0, ci.size()) ci[i] = cfig[i] == '1' ? true : false;

	return move(ci);
}

/*  version 1
array<UInt,6> StateStatistics::cfg2ex2nums(Int ex_pos) {
	// UInt nums[occ_n.nrows()];
	array<UInt,6> nums;
	MatOnb cf_temp(cfg.cf.mat(occ_n.nrows(), occ_n.ncols()));
	Idx pos(ABS(ex_pos) - 1);
	// if(ex_pos > 0) cf_temp = cf_temp[pos][0].crt(0);
	// if(ex_pos < 0) cf_temp = cf_temp[pos][0].ann(0);
	cf_temp[pos][0] = ex_pos > 0 ? cf_temp[pos][0].crt(0) : cf_temp[pos][0].ann(0);
		
	const VecOnb cf(cf_temp.vec());
	for_Idx(i, 0, cf.size()){
		Str cfig;
		for_Int(j, 0, cf[i].get_ns()) cfig += cf[i][j];
		const UInt num {stoul(cfig, nullptr, 2)};
		nums[i] = num;
	}
	return move(nums);
}
*/

//---------------------------------------------Private function---------------------------------------------

//For the assertion that the imp is in the first division[0].
//Int StateStatistics::anticommu(const Int& pi, const Int& pj)
//{
//	Int gap;
//	gap = ABS(pj - pi - 1);
////	if (pi == 0 || pj == 0)	gap = ABS(pj - pi - 1) - 1;//Here discriminate the imp and bath
////	else gap = ABS(pj - pi - 1);
////#ifdef _ASSERTION_
////	if (pj == 0 && pi == 0) ERR("There are some thing wrong with the IDX" + NAV3(gap, pi, pj));
////#endif
//	if ((gap % 2) == 0) return +1;
//	return -1;
//}

ComDivs StateStatistics::cfgdetail()
{
	return ComDivs(idx, (occ_n), (space.sit_mat));
}

void StateStatistics::statistics()
{
	Int sit_spinless(0);
	// spinless orbits
	for_Int(n, 0, filled_spinless.size())
	{
		for_Int(i, n * space.ndivs, (n + 1) * space.ndivs)
		{
			for_Int(j, 0, cfg.cf[i].get_ns())
			{
				if (cfg.cf[i].isocc(j))
					filled_spinless[n].push_back(sit_spinless);
				sit_spinless++;
			}
		}
	}
	// // down site
	// for_Int(i, space.ndivs, space.ndivs + space.ndivs) {
	// 	for_Int(j, 0, cfg.cf[i].get_ns()) {
	// 		if (cfg.cf[i].isocc(j)) filled_dw.push_back(sit_spinless);
	// 		sit_spinless++;
	// 	}
	// }
//#ifdef _ASSERTION_
//	if (space.nspa != filled_up.size() + filled_dw.size()) {
//		ERR("TOTAL partical counting wrong   " + NAV3(space.nspa, filled_up.size(), filled_dw.size()));
//	}
//#endif
}


