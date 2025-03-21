#pragma once

/*
code developed and maintained by (jmw@ruc.edu.cn, RUC, China) date 2022 - 2024
*/

#include "specs.h"
#include "prmtr.h"
#include "impurity.h"
// impurity model
// At the Shortcut space(NocSpace) we set first divison as impurity, and the active orbital(bath site) on the middle.  

typedef Vec<VEC<Int>> Tab;
class NocSpace {

private:
	const BnmlFast bf;
public:
	const MyMpi &mm;				// parameters
	const Prmtr& p;					// parameters
	Idx ndivs;						// The amount of divisons's number. 
	MatInt control_divs;			// to set the number of division and the shortcut restratin.
	VecInt orbital_divcnt;			// the SUM in row for each colum.
	MatInt sit_mat;					// spinless - orbits number in each division.
	Idx nspa;						// The amount of partical's number.
	VecInt nppso;					// nppso mean: number of partical per spin orbital.
	
	Idx dim;						// the dimension of the Shortcut space.
	VEC<MatInt> div;				// a set of combined number.
	std::map<std::string, Idx> divs_to_idx;	// a set of combined number.
	VEC<Int> idx_div;				// a set of idx(The idx of the begining with 0) for each subspace.
	VecReal mu;						// -\mu_{k}^{\prime}b_{k,\sigma}^{+}b_{k,\sigma}
	VEC<VecReal> t_ose;				// H_0 onset energy
	VEC<VecReal> t_hyb;				// hopping parameter from bath to imp.
	MatReal hopint;					// transformed hopping integral
	VecReal coefficient;			// coefficient for all the H's terms, consturct by t_ose and t_hyb.
	
	
	Real u_hbd;						// The Hubbard term U.
	Real j_ob;						// The Hunter couping term J.
	
private:
	void set_control();
	// Find all the combined number subspaces.
	void find_combined_number_subspaces(const Int mode = 0);
	void find_combined_number_subspaces_no_active_orbital();
	// // Find all the combined number subspaces by nppso.
	// void find_combined_number_subspaces(const VecInt& nppso);

	// Find all the combined number subspaces, with speed up.
	void find_all_noc_subspaces();
	void find_all_noc_subspaces_multi();
	void find_all_noc_subspaces_by_row();
	void find_thought_noc_subspaces();
	void find_thought_noc_subspaces_nestedloops();


	void print(std::ostream& os, const Str& var, const Str& val, const Str& comment) const {
		using namespace std;
		Str true_var = var == "\"\"" ? "" : var;
		os << rght_justify(true_var, 16) << (true_var == "" ? "   " : " = ") << left_justify(val, w_Real)
			<< "    # " + comment << endl;
	}
	bool if_div_in_restraint(const VecInt& restraint, const Int position, const Int max, const Int now) const;
	
	bool if_col_divs_in_restraint(const Int& restraint, const VEC<Int>& divcol_i, Int col_idx) const;
	bool if_row_divs_in_restraint(const Int& restraint, const VEC<Int>& divrow_i, VecInt count_sit_mat) const;

	bool ifin_NocSpace_judge_by_nppso(const MatInt& spilss_div, const VecInt& nppso) const;

	VecInt multi_judger(const VEC<VEC<int> >& s, const VEC<VEC<int> >& a) const;
	VecInt multi_judger_by_row(const VEC<VEC<int> >& s, const VEC<VEC<int> >& a) const;
	Vec<MatInt> multi_judger_with_return(VEC<VEC<int> >& s, const VEC<VEC<int> >& a) const;

	bool check_each_column(const Int& col_pos, const VecInt& div_colsum) const {
		if (p.if_norg_imp) {
			if (col_pos < div_colsum.size() / 2 && div_colsum[col_pos] >= orbital_divcnt[col_pos] + control_divs[0][col_pos] && div_colsum[col_pos] <= orbital_divcnt[col_pos]) return true;
			if (col_pos >= div_colsum.size() / 2 && div_colsum[col_pos] >= 0 && div_colsum[col_pos] <= control_divs[0][col_pos]) return true;
		}
		else {
			if (col_pos == div_colsum.size() / 2) return true;
			if (col_pos < div_colsum.size() / 2 && div_colsum[col_pos] >= orbital_divcnt[col_pos] + control_divs[0][col_pos] && div_colsum[col_pos] <= orbital_divcnt[col_pos]) return true;
			if (col_pos > div_colsum.size() / 2 && div_colsum[col_pos] >= 0 && div_colsum[col_pos] <= control_divs[0][col_pos]) return true;
		}
		return false;
	}

	bool check_correlated_column(const Int& col_pos, const VecInt& div_colsum) const;

	// reference the 10.1103/PhysRevB.96.085139 FIG.1 algorithm
	bool check_if_PHSs(const VecInt& div_colsum) const;	
	bool check_if_PHSs_v2(const VecInt& div_colsum) const;

	VecInt read_from_col_lable(const VEC<Int> x, const VEC<VEC<Int> > a) const;

	void find_all_possible_state(VEC<VEC<Int> >& a, VEC<VEC<Int> >& s) const;
	void find_all_possible_state_by_col(VEC<VEC<Int> >& a, VEC<VEC<Int> >& s) const;
	void find_all_possible_state_by_row(VEC<VEC<Int> >& a, VEC<VEC<Int> >& s) const;
	void find_all_possible_state_by_nooc(VEC<VEC<Int> >& a, VEC<VEC<Int> >& s) const;
	void find_all_possible_state_suit_for_PHSs(VEC<VEC<Int> >& a, VEC<VEC<Int> >& s) const;

	
	void nestedLoops(int depth, int n, MatInt control_divs, std::vector<int>& current);
	template<typename T>
	VEC<VEC<T>> cart_product(const VEC<VEC<T>>& v) const {
		VEC<VEC<T>> result = {{}};
		for (const auto& u : v) {
			
			VEC<VEC<T>> temp;
			for (const auto& x : result) {
				for (const auto y : u) {
					auto new_combination = x; // Create a new combination
					new_combination.push_back(y); // Add current element
					temp.push_back(std::move(new_combination)); // Move to temp
				}
			}
			
			// Move temp to result
			result = std::move(temp);
		}
		return result;
	};
	VEC<VEC<Int> > cart_product_monitor_col(const VEC<VEC<int> >& v, const VEC<VEC<Int> >& a)const;
	VEC<VEC<Int> > cart_product_monitor_row(const VEC<VEC<int> >& v, const VEC<VEC<int> >& a) const;
	VEC<VEC<Int> > cart_product_monitor_PHS(const VEC<VEC<int> >& v, const VEC<VEC<int> >& a) const;

	Idx read_the_Tab(Str name) const{
		Idx temp_dim(-1);	
		IFS ifs(STR(name + ".inf"));	Str strr;
		while(1) {// read the Tab's size's info
			ifs >> strr;
			if(strr == "dim")	ifs >> temp_dim;
			if (!ifs) break;
		}
		// WRN(NAV(temp_dim))
		return temp_dim;
	}
	
public:

	// Expand the Shortcut space under Number of particles(NumberSpa).
	NocSpace(const MyMpi& mm_i, const Prmtr& prmtr_i): mm(mm_i), p(prmtr_i) {};
	// NocSpace(const MyMpi& mm_i, const Prmtr& prmtr_i, const Int& NumberSpa);
	NocSpace(const MyMpi& mm_i, const Prmtr& prmtr_i, const Int& NumberSpa);
	NocSpace(const MyMpi& mm_i, const Prmtr& prmtr_i, const VecInt& nppso_i);
	NocSpace(const MyMpi& mm_i, const Prmtr& prmtr_i, const VecInt& nppso_i, Str tab_name);
	NocSpace(const MyMpi& mm_i, const Prmtr& prmtr_i, const VecInt& nppso_i, const Tab& tab);
	
	// bool ifin_NocSpace(VecInt& ud) const;
	bool ifin_NocSpace(MatInt& ud) const;
	// bool ifin_NocSpace_for_green(MatInt& spilss_div, const VecInt& nppso, const Int& crtorann) const;
	bool ifin_NocSpace(MatInt& spilss_div, const VecInt& nppso) const;
	bool suit_NOOC(MatInt& spilss_div, const VecInt& nppso) const;
	
	bool ifin_NocSpace(const VecBool new_cfig, const VecInt& nppso) const;

	bool ifin_NocSpace_more_strict(MatInt& spilss_div, const VecInt& nppso) const;

	Int wherein_NocSpace(const Int& h_i) const;

	void print(std::ostream& os = std::cout) const;

	Str nppso_str() const	{
		Str temp; for_Int(i, 0, p.npartical.size()) { if (i == 0) temp += STR(p.npartical[i]); else /*if (i % 2 == 0)*/ temp += "-" + STR(p.npartical[i]); }	return temp;
	}

	VecInt free_div_base_decode(Idx idx, VEC<VEC<Int> > v) const;
};



/* // NOT in used code

/// #1 cart_product
	template<typename T>
	VEC<VEC<T>> cart_product(const VEC<VEC<T>> &v) const
	{
		VEC<VEC<T>> s = {{}};
		for (const auto &u : v)
		{
			VEC<VEC<T>> r;
			for (const auto &x : s)
			{
				for (const auto y : u)
				{
					r.push_back(x);
					r.back().push_back(y);
				}
			}
			s = move(r);
		}
		return s;
	};
///



*/