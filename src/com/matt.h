/*@
Copyright (c) 2013-2014, Su Zhenyu steven.known@gmail.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Su Zhenyu nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@*/
#ifndef __MATT_H__
#define __MATT_H__
/*
START MATRIX

Series of functions manipulate matrix.
*/
#define HOOK_INIT m_inhr.hi
#define HOOK_EQUAL m_inhr.he
#define HOOK_ADJUST m_inhr.ha
#define HOOK_SQRT m_inhr.hs
#define HOOK_DUMPS m_inhr.hds
#define HOOK_DUMPF m_inhr.hdf
#define HOOK_DUMPFH m_inhr.hdfh

/*
NOTICE:
	1. We want to abstract these objects which have same operations
		in '+' '-' '*' '/' '<' '<=' '>' '>=' '!=' '==' 'minus'.
		These objects may be integer, real number, complex number, vector,
		matrix, etc.
 */
#define MAX_MAT_RANK		512
#define MAX_QR_ITER_NUM		512
//#define QR_ITER_WITH_SHIFT  //low precision
#define NORM_1				1 //1 norm
#define NORM_2				2 //2 norm
#define NORM_F				3 //Frobenius norm
#define NORM_INF			4 //infinite norm

/*
Multiplification: a * b
*/
template <class T> class MATRIX;
template <class T>
MATRIX<T> operator * (MATRIX<T> const& a, MATRIX<T> const& b)
{
	IS_TRUE(a.m_is_init && b.m_is_init,
			("not yet initialize."));
	IS_TRUE(a.m_row_size > 0 && a.m_col_size > 0, ("invalid matrix"));
	IS_TRUE(b.m_row_size > 0 && b.m_col_size > 0, ("invalid matrix"));
	IS_TRUE(a.m_col_size == b.m_row_size, ("invalid matrix type of mul"));
	IS_TRUE(a.HOOK_INIT == b.HOOK_INIT &&
			a.HOOK_EQUAL == b.HOOK_EQUAL,
			("strange matrix member function"));
	MATRIX<T> c(a.m_row_size, b.m_col_size, &a.m_inhr);
	for (UINT i = 0; i < a.m_row_size; i++) {
		for (UINT j = 0; j < b.m_col_size; j++) {
			T tmp = 0;
			for (UINT k = 0; k < a.m_col_size; k++) {
				tmp = tmp + a.get(i,k) * b.get(k,j);
			}
			c.set(i,j,tmp);
		}
	}
	return c;
}


/*
Addition: a + b
*/
template <class T>
MATRIX<T> operator + (MATRIX<T> const& a, MATRIX<T> const& b)
{
	IS_TRUE(a.m_is_init && b.m_is_init,
			("not yet initialize."));
	IS_TRUE(a.m_row_size == b.m_row_size && a.m_col_size == b.m_col_size,
			("invalid matrix type of mul"));
	IS_TRUE(a.HOOK_INIT == b.HOOK_INIT &&
			a.HOOK_EQUAL == b.HOOK_EQUAL,
			("strange matrix member function"));
	MATRIX<T> c(a.m_row_size, a.m_col_size, &a.m_inhr);
	for (UINT i = 0; i < a.m_row_size; i++) {
		for (UINT j = 0; j < a.m_col_size; j++) {
			c.set(i,j,a.get(i,j) + b.get(i,j));
		}
	}
	return c;
}


/*
Subtraction: a - b
*/
template <class T>
MATRIX<T> operator - (MATRIX<T> const& a, MATRIX<T> const& b)
{
	IS_TRUE(a.m_is_init && b.m_is_init,
			("not yet initialize."));
	IS_TRUE(a.m_row_size == b.m_row_size && a.m_col_size == b.m_col_size,
			("invalid matrix type of mul"));
	IS_TRUE(a.HOOK_INIT == b.HOOK_INIT &&
			a.HOOK_EQUAL == b.HOOK_EQUAL,
			("strange matrix member function"));
	MATRIX<T> c(a.m_row_size, a.m_col_size, &a.m_inhr);
	for (UINT i = 0; i < a.m_row_size; i++) {
		for (UINT j = 0; j < a.m_col_size; j++) {
			c.set(i,j,a.get(i,j) - b.get(i,j));
		}
	}
	return c;
}


template <class T> class MATRIX {
public:
	//Intra-class type, only used by MATRIX and inherited classes.
	typedef void (*HOOK_INIT_FUNPTR)(MATRIX<T> * pbasis);

	//These hooks should NOT modify any other global variables!
	typedef bool (*HOOK_EQUAL_FUNPTR)(T const& a, T const& b);
	typedef void (*HOOK_ADJUST_FUNPTR)(MATRIX<T> * pbasis);
	typedef T (*HOOK_SQRT_FUNPTR)(T t);
	typedef void (*HOOK_DUMPS_FUNPTR)(void const* pbasis);
	typedef void (*HOOK_DUMPF_FUNPTR)(void const* pbasis, CHAR const* name, bool is_del);
	typedef void (*HOOK_DUMPFH_FUNPTR)(void const* pbasis, FILE * h);

	//Packing initializing information for inherit class.
	typedef struct {
		HOOK_INIT_FUNPTR hi;
		HOOK_EQUAL_FUNPTR he;
		HOOK_ADJUST_FUNPTR ha;
		HOOK_SQRT_FUNPTR hs;
		HOOK_DUMPS_FUNPTR hds;
		HOOK_DUMPF_FUNPTR hdf;
		HOOK_DUMPFH_FUNPTR hdfh;
	} INHR;

	INHR m_inhr;
	UINT m_row_size;   //row size
	UINT m_col_size;   //column size
	T * m_mat;
	bool m_is_init;

	T _full_permutation(UINT n);
	void _full_permutation_recur(INT v, INT * posbuf, UINT posbufnum, INT n, T & det);
	INT _reverse_order_number(INT * numbuf, UINT numlen);
	bool _equal(T const& a, T const& b) const;
	T _sqrt(T v);
public:
	MATRIX(INHR const* i = NULL);
	MATRIX(const MATRIX<T> & m);
	MATRIX(UINT row, UINT col, INHR const* i = NULL);
	~MATRIX();
	void set_hook(INHR const* i);
	void init(INHR const* i = NULL);
	void init(UINT row, UINT col, INHR const* i = NULL);
	void reinit(UINT row, UINT col, INHR const* i = NULL);
	void destroy();

	void clean(); //Eliminating all elements.
	UINT count_mem() const;

	T abs(T v) const;

	UINT get_row_size() const;
	UINT get_col_size() const;
	T * get_matrix() const;
	void grow_row(UINT size);
	void grow_row(const T row[], UINT rowelemnum);
	void grow_row(SVECTOR<T> const& row, UINT rowelemnum);
	void grow_row(MATRIX<T> const& a); //Add row, a must be vector

	//Add row from 'from' to 'to' of a.
	void grow_row(MATRIX<T> const& a, UINT from, UINT to);
	void grow_col(UINT size);
	void grow_col(const T col[], UINT colelemnum);
	void grow_col(SVECTOR<T> const& col, UINT colelemnum);
	void grow_col(MATRIX<T> const& a); //Add column, a must be vector

	//Append columns from 'from' to 'to' of a.
	void grow_col(MATRIX<T> const& a, UINT from, UINT to);
	void grow_all(UINT row_size, UINT col_size);
	void del_row(UINT row);
	void del_row(UINT from, UINT to);
	void del_col(UINT col);
	void del_col(UINT from, UINT to);
	void pad_quad(); //pad row and col to generate quad matrix

	UINT size() const; //Number of elements
	void set(UINT row, UINT col, T v);
	void setg(UINT row, UINT col, T v);
	void set_all(T v); //Set all element to 'v'
	void set_row(UINT rows, T row[], INT num);
	void set_row(UINT rows, IN MATRIX<T> const& v);
	void set_row(UINT rows, T v);
	void set_rows(UINT from, UINT to, MATRIX<T> const& m, UINT mfrom);
	void set_rows(UINT from, UINT to, T v);
	void set_col(UINT cols, T col[], INT num);
	void set_col(UINT cols, IN MATRIX<T> const& v);
	void set_col(UINT cols, T v);
	void set_cols(UINT from, UINT to, MATRIX<T> const& m, UINT mfrom);
	void set_cols(UINT from, UINT to, T v);
	T get(UINT row, UINT col) const;
	void interch_col(UINT col1, UINT col2);
	void interch_row(UINT row1, UINT row2);
	void copy(MATRIX<T> const& m);
	void trans(); //Transpose

	//Solve system of equaions, 'this' is coeff matirx
	bool sse(OUT MATRIX<T> & x, IN MATRIX<T> const& b);

	//Method of least squares
	void mls(OUT MATRIX<T> & x, IN MATRIX<T> const& b);
	MATRIX<T> & operator = (MATRIX<T> const& m);
	bool operator == (MATRIX<T> const& m) const;
	bool operator != (MATRIX<T> const& m) const;
	bool is_quad() const; //matrix is square
	bool is_unitary() const; //matrix is I
	bool is_symm() const; //matrix is symmetry
	bool is_orth() const; //matrix is orthogonal
	bool is_nonsig() const; //matrix is nonsingular
	bool is_homo(MATRIX<T> const& m) const;
	bool is_rowvec() const; //matrix is row vector
	bool is_colvec() const; //matrix is col vector
	bool is_rowequ(UINT row, T v) const; //row entry equals 'v'.
	bool is_rowequ(UINT row, IN MATRIX<T> const& m, IN UINT mrow) const;
	bool is_colequ(UINT col, T v) const; //col entry equals 'v'.
	bool is_colequ(UINT col, IN MATRIX<T> const& m, IN UINT mcol) const;
	bool is_vec() const; //matrix is vector
	bool is_low_triangular() const;
	bool is_anti_low_triangular() const;
	bool is_up_triangular() const;
	bool is_anti_up_triangular() const;
	//Add row 'from' to row 'to', then row 'to' be modified
	void add_row_to_row(UINT from, UINT to);
	void add_row_to_row(IN MATRIX<T> const& m, IN UINT mfrom, IN UINT to);
	void add_col_to_col(UINT from, UINT to);
	void add_col_to_col(IN MATRIX<T> const& m, IN UINT mfrom, IN UINT to);

	void mul(T v); //scalar multiplification
	//Scalar multiplification for each element of row
	void mul_of_row(UINT row, T v);
	void mul_of_rows(UINT from, UINT to, T v);
	//scalar multiplification for each element of col
	void mul_of_col(UINT col, T v);
	void mul_of_cols(UINT from, UINT to, T v);
	void neg();
	void neg_of_rows(UINT from, UINT to);
	void neg_of_cols(UINT from, UINT to);

	//v times row 'from', then add row to 'to'.
	void mul_and_add_row(UINT from, T v, UINT to);
	void mul_and_add_col(UINT from, T v, UINT to);
	bool inv(OUT MATRIX<T> & e); //inverse of matrix
	bool rinv(OUT MATRIX<T> & e); //right-inverse
	bool linv(OUT MATRIX<T> & e); //left-inverse
	void refinv(OUT MATRIX<T> & e); //reflexive g-inverse
	void minv(OUT MATRIX<T> & x); //least-norm inverse
	void lstinv(OUT MATRIX<T> & x); //least-square inverse
	bool ginv(OUT MATRIX<T> & e); //{1}-inverse

	//Generalized inverse, namely Moore-Penrose inverse
	void pinv(OUT MATRIX<T> & e);
	void eye(T v); //Initialize to identical matrix with pivot entry 'v'
	void zero(); //Set each entry to zero
	void fzero(); //Fast zeroization, memset(0) invoked
	void eche(); //Reduce matrix to row-echelon normal form.

	//Calculate null space(kern), each column of 'ns' indicate every unknown.
	void null(MATRIX<T> & ns);
	void basis(OUT MATRIX<T> & b); //Calculate basis

	//Calculate algebraic complement
	void algc(UINT row, UINT col, MATRIX<T> & m);
	bool adj(MATRIX<T> & m); //Calculate adjoint

	//Initialize diagonal entry to 'v'
	void setdiag(IN SVECTOR<T> const& v, UINT vlen);

	//Initialize diagonal entry to 'v'
	void setdiag(IN MATRIX<T> const& v);

	//Set 'v' to row vector by diagonal entry.
	void getdiag(OUT MATRIX<T> & v);
	UINT rank(MATRIX<T> * basis = NULL, bool is_unitarize = false);
	T tr(); //Computation of trace
	T det() const;
	T mod(); //Modular of vector, 'this' must be vector
	T modrow(UINT row); //Modular of row vector
	T modcol(UINT col); //Modular of col vector
	void eig(OUT MATRIX<T> & eigv); //Eigenvalues and eigenvectors
	void eig(OUT MATRIX<T> & eigv, OUT MATRIX<T> & eigx);
	void cross(IN MATRIX<T> & v, OUT MATRIX<T> & u); //vector product.
	T dot(IN MATRIX<T> const& v) const;
	T dot(UINT srow, UINT scol, UINT erow,
			UINT ecol, IN MATRIX<T> const& v) const;
	T dot(UINT srow, UINT scol, UINT erow, UINT ecol,
				IN MATRIX<T> const& v, UINT vsrow, UINT vscol,
				UINT verow, UINT vecol) const;
	T dotrow(UINT row, IN MATRIX<T> const& v) const;
	T dotcol(UINT col, IN MATRIX<T> const& v) const;
	void insert_row_before(UINT ridx);
	void insert_rows_before(UINT ridx, UINT rnum);
	void insert_rows_before(UINT ridx, IN MATRIX<T> const& m,
							UINT mfrom, UINT mto);
	void insert_col_before(UINT cidx);
	void insert_cols_before(UINT cidx, UINT cnum);
	void insert_cols_before(UINT cidx, IN MATRIX<T> const& m,
							UINT mfrom, UINT mto);

	//Get inner matrix
	void inner(MATRIX<T> & in, UINT sr, UINT sl, UINT er, UINT el) const;

	//Get inner row of matrix
	void inner_row(OUT MATRIX<T> & in, UINT from, UINT to) const;

	//Get inner row of matrix
	void inner_col(OUT MATRIX<T> & in, UINT from, UINT to) const;

	//pivot triangular decomposition
	bool plu(MATRIX<T> & p, MATRIX<T> & l, MATRIX<T> & u);

	//Non-pivot triangular decomposition
	bool lu(MATRIX<T> & l, MATRIX<T> & u);
	void nml(); //Normalize each row vector

	//Orthogonal decomposition
	void qr(MATRIX<T> & q, MATRIX<T> & r, bool calc_basis = false);

	//Full rank decomposition
	void frd(OUT MATRIX<T> & f, OUT MATRIX<T> & g);

	//Pivot orthogonal decomposition
	bool pqr(MATRIX<T> & p, MATRIX<T> & q, MATRIX<T> & r);
	void orthn(MATRIX<T> & z); //Calculate orthonormal basis
	void orth(MATRIX<T> & z); //Calculate orthogonal basis
	void adjust(); //Adjust significant digit and revising value.
					//Inherit class should overload this function
					//if it has numberical error.

	//Computation of projection of vector 'v' in row space of 'this'.
	void proj(OUT MATRIX<T> & proj, IN MATRIX<T> const& p);

	//Stranger value decomposition.
	bool svd(OUT MATRIX<T> & u, OUT MATRIX<T> & s, OUT MATRIX<T> & eigx);
	bool diag(OUT MATRIX<T> & p, OUT MATRIX<T> & d); //Diagonalize matrix
	T norm(INT p); //Calculate matrix/vector norm.
	bool cond(T & c, INT p = NORM_INF); //Calculate condition number.
	T sprad(); //Calculate spectral radius.
	void padding(); //complement to nonsingular
	//T integral(FUNCTY s, INT vidx, T lowb, T upperb);

	//Print matrix element to file.
	void dumpf(FILE * h) const;
	void dumpf(CHAR * name = NULL, bool is_del = false) const;
	void dumps() const; //Print matrix element on screen.
};


template <class T>
MATRIX<T>::MATRIX(INHR const* i)
{
	m_is_init = false;
	init(i);
}


//Copy constructor
template <class T>
MATRIX<T>::MATRIX(MATRIX<T> const& m)
{
	m_is_init = false;
	init();
	copy(m);
}


template <class T>
MATRIX<T>::MATRIX(UINT row, UINT col, INHR const* i)
{
	m_is_init = false;
	init(row, col, i);
	//C++ does not call constructor for non-POD
	//type when funtion invoking with '...' parameters.
	/*
	T * pv = (T*)(((CHAR*)&col) + sizeof(INT));
	INT vlen = row * col;
	UINT i = 0;
	while (i < vlen) {
		//T v = va_arg(arg, T);
		m_mat[i] = *pv;
		i++;
		pv++;
	}
	*/
}


template <class T>
MATRIX<T>::~MATRIX()
{
	destroy();
}


template <class T>
void MATRIX<T>::init(INHR const* i)
{
	if (m_is_init) return;
	init(0, 0, i);
}


template <class T>
void MATRIX<T>::set_hook(INHR const* i)
{
	if (i) {
		m_inhr = *i;
	} else {
		memset(&m_inhr, 0, sizeof(INHR));
	}
}


template <class T>
void MATRIX<T>::init(UINT row, UINT col, INHR const* i)
{
	if (m_is_init) return;
	m_row_size = 0;
	m_col_size = 0;
	m_mat = NULL;
	m_is_init = true;
	set_hook(i);
	grow_all(row, col);
	if (HOOK_INIT != NULL) {
		(*HOOK_INIT)(this);
	}
}


template <class T>
void MATRIX<T>::reinit(UINT row, UINT col, INHR const* i)
{
	INHR inhr;
	if (i != NULL) {
		inhr = *i;
	} else {
		inhr = m_inhr;
	}
	destroy();
	init(row, col, &inhr);
}


template <class T>
void MATRIX<T>::destroy()
{
	if (!m_is_init) return;
	m_row_size = 0;
	m_col_size = 0;
	if (m_mat != NULL) {
		::free(m_mat);
	}
	m_mat = NULL;
	set_hook(NULL);
	m_is_init = false;
}


//Destructing all elements.
template <class T>
void MATRIX<T>::clean()
{
	if (!m_is_init) return;
	m_row_size = 0;
	m_col_size = 0;
	if (m_mat != NULL) {
		::free(m_mat);
	}
	m_mat = NULL;
}


template <class T>
UINT MATRIX<T>::count_mem() const
{
	UINT count = 0;
	count += sizeof(m_inhr);
	count += sizeof(m_row_size);
	count += sizeof(m_col_size);
	count += sizeof(m_mat);
	count += sizeof(m_is_init);
	count += size() * sizeof(T);
	return count;
}


template <class T>
T MATRIX<T>::abs(T v) const
{
	if (v < 0) {
		v = -(v);
	}
	return v;
}


/*
Interior equal compare.
 */
template <class T>
bool MATRIX<T>::_equal(T const& a, T const& b) const
{
	if (HOOK_EQUAL != NULL) {
		return (*HOOK_EQUAL)(a, b);
	}
	return a == b;
}


/*
Interior extraction.
Here only apply an instantiate that simpliest call C/C++ library.
If it is inappropriate, overloading it as your need.
*/
template <class T>
T MATRIX<T>::_sqrt(T v)
{
	if (HOOK_SQRT != NULL) {
		return (*HOOK_SQRT)(v);
	}
	IS_TRUE(0, ("_sqrt() need to implement"));
	return v;
}


/*
Error analysis and elimination.
*/
template <class T>
void MATRIX<T>::adjust()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (HOOK_ADJUST != NULL) {
		(*HOOK_ADJUST)(this);
	}
}


template <class T>
UINT MATRIX<T>::size() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	return m_row_size * m_col_size;
}


/*
Growing row of matrix.

'size': rows to be grown.

NOTICE:
	If matrix is empty, growing column vector contains 'size' rows.
*/
template <class T>
void MATRIX<T>::grow_row(UINT size)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (size == 0) {
		return;
	}
	UINT oldrows = 0;
	if (m_row_size == 0) {
		//If matrix is empty, growing 'size' rows, but only one column.
		IS_TRUE(!m_col_size && m_mat == NULL, ("matrix should be empty"));
		m_mat = (T*)::malloc(sizeof(T) * size);
		memset(m_mat, 0, sizeof(T) * size);
		m_col_size = 1;
		m_row_size = size;
		goto INIT;
	}
	{
		UINT l = m_col_size * (m_row_size + size) * sizeof(T);
		T * tmp_mat = (T*)::malloc(l);
		memcpy(tmp_mat, m_mat, m_col_size * m_row_size * sizeof(T));
		//memset(tmp_mat + m_col_size * m_row_size, 0,
		//		size * m_col_size * sizeof(T));
		::free(m_mat);
		m_mat = tmp_mat;
		oldrows = m_row_size;
		m_row_size += size;
	}
INIT:
	for (UINT i = oldrows; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			set(i, j, T(0));
		}
	}
}


template <class T>
void MATRIX<T>::grow_row(SVECTOR<T> const& row, UINT rowelemnum)
{
	IS_TRUE(row.m_vec, ("vector is empty"));
	grow_row(row.m_vec, rowelemnum);
}


/*
Growing one row.
'row'  row which should copy to m_mat, and 'num' descripted how
		many columns the row is.
If number of elememnt in 'row' less than m_col_size, set the
		remain entries to zero.
*/
template <class T>
void MATRIX<T>::grow_row(const T row[], UINT rowelemnum)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE0(row != NULL);
	if (rowelemnum == 0) return;
	if (m_row_size == 0) {
		IS_TRUE(!m_col_size && m_mat == NULL, ("matrix should be empty"));
		m_mat = (T*)::malloc(sizeof(T) * rowelemnum);
		memcpy(m_mat, row, sizeof(T) * rowelemnum);
		m_row_size = 1;
		m_col_size = rowelemnum;
		return;
	}
	IS_TRUE(rowelemnum <= m_col_size,("unmatch column sizes"));
	INT l = m_col_size * (m_row_size + 1) * sizeof(T);
	T * tmp_mat = (T*)::malloc(l);
	memcpy(tmp_mat, m_mat, m_col_size * m_row_size * sizeof(T));
	memcpy(tmp_mat + m_col_size * m_row_size, row, rowelemnum * sizeof(T));
	if (rowelemnum < m_col_size) {
		memset(tmp_mat + (m_col_size * m_row_size + rowelemnum), 0,
				(m_col_size - rowelemnum) * sizeof(T));
	}
	::free(m_mat);
	m_mat = tmp_mat;
	m_row_size++;
}


//Add row from a vector 'a'.
template <class T>
void MATRIX<T>::grow_row(MATRIX<T> const& a)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(a.is_vec(), ("a must be vector"));
	if (m_row_size > 0) {
		IS_TRUE(a.size() == m_col_size, ("invalid vector"));
	}
	if (m_row_size == 0) {
		IS_TRUE(!HOOK_INIT || HOOK_INIT == a.HOOK_INIT, ("unmatch hook"));
		IS_TRUE(!HOOK_EQUAL || HOOK_EQUAL == a.HOOK_EQUAL, ("unmatch hook"));
		m_inhr = a.m_inhr;
	}
	grow_row(a.m_mat, a.size());
}


//Add row from 'from' to 'to' of a.
template <class T>
void MATRIX<T>::grow_row(MATRIX<T> const& a, UINT from, UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(from < a.m_row_size && to < a.m_row_size && from <= to,
			("not yet initialize."));
	if (m_row_size == 0) {
		//If matrix is empty, growing 'size' rows, but only one column.
		IS_TRUE(!m_col_size && m_mat == NULL, ("matrix should be empty"));
		m_col_size = a.m_col_size;
		m_row_size = to - from + 1;
		m_mat = (T*)::malloc(sizeof(T) * m_row_size * m_col_size);
		memcpy(m_mat, a.m_mat + from * a.m_col_size,
				sizeof(T) * m_row_size * m_col_size);
		return;
	}
	IS_TRUE(a.m_col_size == m_col_size, ("unmatch matrix"));
	INT row_size = to - from + 1;
	T * tmp_mat = (T*)::malloc((m_row_size + row_size) *
								m_col_size * sizeof(T));
	IS_TRUE(tmp_mat,("memory is low!!!"));
	UINT orig_size = m_col_size * m_row_size;
	memcpy(tmp_mat, m_mat, orig_size * sizeof(T));
	memcpy(tmp_mat + orig_size, a.m_mat + from * a.m_col_size,
			row_size * m_col_size * sizeof(T));
	::free(m_mat);
	m_mat = tmp_mat;
	m_row_size += row_size;
}


/*
Growing column of matrix.

'size': columns to be grown.

NOTICE:
	If matrix is empty, growing column vector contains a
	number of 'size' of columns.
*/
template <class T>
void MATRIX<T>::grow_col(UINT size)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (size == 0) {
		return;
	}
	if (m_col_size == 0) { //matrix is empty
		//If matrix is empty, growing 'size' columns, but only one row.
		IS_TRUE(!m_row_size &&
				m_mat == NULL, ("exception occur in grow_col()"));
		m_mat = (T*)::malloc(sizeof(T) * size);
		memset(m_mat, 0, sizeof(T) * size);
		m_col_size = size;
		m_row_size = 1;

		//Initializing
		for (UINT j = 0; j < m_col_size; j++) {
			set(0, j, T(0));
		}
		return;
	}
	grow_all(0, size);
}


template <class T>
void MATRIX<T>::grow_col(SVECTOR<T> const& col, UINT colelemnum)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(col.m_vec, ("vector is empty"));
	grow_col(col.m_vec, colelemnum);
}


//Add column from a vector 'a'.
template <class T>
void MATRIX<T>::grow_col(MATRIX<T> const& a)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(a.is_vec(), ("a must be vector"));
	if (m_col_size > 0) {
		IS_TRUE(a.size() == m_row_size, ("invalid vector"));
	}
	if (m_col_size == 0) {
		IS_TRUE(!HOOK_INIT || HOOK_INIT == a.HOOK_INIT, ("unmatch hook"));
		IS_TRUE(!HOOK_EQUAL || HOOK_EQUAL == a.HOOK_EQUAL, ("unmatch hook"));
		m_inhr = a.m_inhr;
	}
	grow_col(a.m_mat, a.size());
}


//Add column from 'sl' to 'el' of 'a'.
template <class T>
void MATRIX<T>::grow_col(MATRIX<T> const& a, UINT from, UINT to)
{
	IS_TRUE(m_is_init && a.m_is_init, ("not yet initialize."));
	IS_TRUE(from < a.m_col_size && to < a.m_col_size && from <= to,
			("not yet initialize."));

	if (m_col_size == 0) { //matrix is empty

		//m_row_size also be 0
		IS_TRUE(!m_row_size && m_mat == NULL,
				("exception occur in grow_all()"));
		m_row_size = a.m_row_size;
		m_col_size = to - from + 1;
		m_mat = (T*)::malloc(sizeof(T) * m_row_size * m_col_size);
		for (UINT i = 0 ; i < m_row_size; i++) {
			memcpy(m_mat + i * m_col_size,
					a.m_mat + i * a.m_col_size + from,
					sizeof(T) * m_col_size);
		}
		return;
	}

	IS_TRUE(a.m_row_size == m_row_size, ("unmatch matrix"));
	INT col_size = to - from + 1;
	T * tmp_mat = (T*)::malloc(m_row_size *
							(m_col_size + col_size) * sizeof(T));
	IS_TRUE(tmp_mat,("memory is low!!!"));
	for (UINT i = 0 ; i < m_row_size; i++) {
		memcpy(tmp_mat + i * (m_col_size+col_size),
				m_mat + i * m_col_size,
				m_col_size * sizeof(T));
		memcpy(tmp_mat + i * (m_col_size+col_size) + m_col_size,
				a.m_mat + i * a.m_col_size + from,
				sizeof(T) * col_size);
	}
	::free(m_mat);
	m_mat = tmp_mat;
	m_col_size += col_size;
}


/*
Growing one col.
'col'  col which should copy to m_mat, and 'colelemnum' descripted
	how many row the column is
*/
template <class T>
void MATRIX<T>::grow_col(const T col[], UINT colelemnum)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE0(col != NULL);
	if (m_col_size == 0) {
		IS_TRUE(!m_row_size && m_mat == NULL,
				("exception occur in grow_col()"));
		m_mat = (T*)::malloc(sizeof(T) * colelemnum);
		memcpy(m_mat, col, sizeof(T) * colelemnum);
		m_row_size = colelemnum;
		m_col_size = 1;
		return;
	}
	IS_TRUE(colelemnum <= m_row_size,("unmatch column sizes"));
	UINT s = m_row_size * (m_col_size+1) * sizeof(T);
	T * tmp_mat = (T*)::malloc(s);
	IS_TRUE(tmp_mat,("memory is low!!!"));
	for (UINT i = 0 ; i < m_row_size; i++) {
		memcpy(tmp_mat + i * (m_col_size + 1), m_mat + i * m_col_size,
						  m_col_size * sizeof(T));
		if (i >= colelemnum) {
			*(tmp_mat + i*(m_col_size+1) + m_col_size) = 0;
		} else {
			*(tmp_mat + i*(m_col_size+1) + m_col_size) = col[i];
		}
	}
	::free(m_mat);
	m_mat = tmp_mat;
	m_col_size++;
}


template <class T>
void MATRIX<T>::grow_all(UINT row_size, UINT col_size)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	UINT i,j;
	if (row_size == 0 && col_size == 0) {
		return;
	}

	//Only grow row
	if (row_size > 0 && col_size == 0) {
		grow_row(row_size);
		return;
	}

	//The complexity of grow column is same as grow both.
	if (m_col_size == 0) { //matrix is empty
		//m_row_size also be 0
		IS_TRUE(!m_row_size && m_mat == NULL,
				("exception occur in grow_all()"));
		m_mat = (T*)::malloc(sizeof(T) * row_size * col_size);
		memset(m_mat, 0 , sizeof(T) * row_size * col_size);
		m_row_size = row_size;
		m_col_size = col_size;
		return;
	}

	UINT newsize = (m_row_size + row_size) *
				(m_col_size + col_size) * sizeof(T);
	T * tmp_mat = (T*)::malloc(newsize);
	IS_TRUE(tmp_mat,("memory is low!!!"));
	//memset(tmp_mat, 0, newsize); //initilzed later on.

	for (i = 0 ; i < m_row_size; i++) {
		memcpy(tmp_mat + i * (m_col_size+col_size),
				m_mat + i * m_col_size, m_col_size * sizeof(T));
	}
	::free(m_mat);
	m_mat = tmp_mat;

	UINT oldrows = m_row_size;
	UINT oldcols = m_col_size;
	m_row_size += row_size;
	m_col_size += col_size;

	//Initlizing elements.
	for (i = 0; i < oldrows; i++) {
		for (j = oldcols; j < m_col_size; j++) {
			set(i, j, T(0));
		}
	}
	for (i = oldrows; i < m_row_size; i++) {
		for (j = 0; j < m_col_size; j++) {
			set(i, j, T(0));
		}
	}
}


template <class T>
void MATRIX<T>::del_row(UINT row)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	del_row(row, row);
}


template <class T>
void MATRIX<T>::del_row(UINT from, UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(from < m_row_size && to < m_row_size && from <= to,
			("out of boundary"));
	if (m_row_size == 0) {
		return;
	}
	UINT l = m_col_size *
			(m_row_size - (to - from + 1)) * sizeof(T);
	T * tmp_mat = (T*)::malloc(l);
	memcpy(tmp_mat, m_mat, m_col_size * from * sizeof(T));
	memcpy(tmp_mat + m_col_size * from,
			m_mat + m_col_size * (to + 1),
			m_col_size * (m_row_size - to - 1) * sizeof(T));
	::free(m_mat);
	m_mat = tmp_mat;
	m_row_size -= (to - from + 1);
}


template <class T>
void MATRIX<T>::del_col(UINT col)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	del_col(col, col);
}


template <class T>
void MATRIX<T>::del_col(UINT from, UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(from < m_col_size && to < m_col_size && from <= to,
			("out of boundary"));
	if (m_col_size == 0) {
		return;
	}
	INT col_size = m_col_size - (to - from + 1);
	UINT l = col_size * m_row_size  * sizeof(T);
	T * tmp_mat = (T*)::malloc(l);
	for (UINT i = 0 ; i < m_row_size; i++) {
		memcpy(tmp_mat + i * col_size,
							m_mat + i * m_col_size,
							from * sizeof(T));
		memcpy(tmp_mat + i * col_size + from,
							m_mat + (i * m_col_size + to + 1),
							(m_col_size - to  - 1)* sizeof(T));
	}
	::free(m_mat);
	m_mat = tmp_mat;
	m_col_size = col_size;
}


/*
Pading to quad matrix, and rank = MAX(row, col)
*/
template <class T>
void MATRIX<T>::pad_quad()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (m_row_size == m_col_size)
		return;
	INT rank = MAX(m_row_size, m_col_size);
	if (m_row_size < m_col_size) {
		grow_row(m_col_size - m_row_size);
	} else {
		grow_col(m_row_size - m_col_size);
	}
}


/*
Get element of matrix

Simulating function like as 'v = m_mat[row][col]'
Because we allocated memory continuously, so we cannot access element
of 'm_mat'by 'm_mat[row][col]' directly.
*/
template <class T>
T MATRIX<T>::get(UINT row, UINT col) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(row < m_row_size && col < m_col_size,
			("exception occur in get()"));
	return *(m_mat + (m_col_size * row + col));
}


/*
Set element of matrix

Simulating function like as 'm_mat[row][col] = v'
Because we allocated memory continuously, so we cannot access element
of 'm_mat' by 'm_mat[row][col]' directly.
*/
template <class T>
void MATRIX<T>::set(UINT row, UINT col, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(row < m_row_size && col < m_col_size,("exception occur in set()"));
	*(m_mat + (m_col_size * row + col)) = v;
}


/*
Set element of matrix.
If element (row,col) out side of current range, grow the matrix to size
of 'row' and 'col' at least. Then set that element to 'v'.
*/
template <class T>
void MATRIX<T>::setg(UINT row, UINT col, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (m_row_size <= row && m_col_size > col) { //grow row
		grow_row(row + 1 - m_row_size);
	} else if (m_row_size > row && m_col_size <= col) { //grow col
		grow_col(col + 1 - m_col_size);
	} else if (m_row_size <= row && m_col_size <= col) { //grow both
		grow_all(row + 1 - m_row_size, col + 1 - m_col_size);
	}
	set(row, col, v);
}


/*
Set all 'row' row elements of matrix
rows: row need to set new value.
*/
template <class T>
void MATRIX<T>::set_row(UINT rows, T row[], INT num)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(rows < m_row_size && num == m_col_size,
			("exception occur in set()"));
	memcpy(m_mat + rows * m_col_size, row, sizeof(T) * m_col_size);
}


/*
Set all 'row' row elements with the element in matrix 'v'
rows: row need to set
v: v must be vector
*/
template <class T>
void MATRIX<T>::set_row(UINT rows, MATRIX<T> const& v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(rows < m_row_size, ("exception occur in set()"));
	IS_TRUE(v.is_vec(), ("v must be vector"));
	IS_TRUE((m_col_size == v.size()), ("invalid vector"));
	if (v.is_rowvec()) {
		IS_TRUE(v.m_col_size == m_col_size,("unmatch vector"));
		memcpy(m_mat + rows * m_col_size, v.m_mat, sizeof(T) * m_col_size);
	} else if (v.is_colvec()) {
		IS_TRUE(v.m_row_size == m_col_size,("unmatch vector"));
		for (UINT i = 0; i < m_col_size; i++) {
			set(rows, i, v.get(i, 0));
		}
	} else {
		IS_TRUE(0, ("invalid operation"));
	}
}


/*
Overlays on elements.
*/
template <class T>
void MATRIX<T>::set_rows(UINT from, UINT to, MATRIX<T> const& m, UINT mfrom)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m.m_col_size == m_col_size &&
			from <= to && to < m_row_size &&
			mfrom + to - from < m.m_row_size,
			("illegal matrix"));
	for (UINT i = from; i <= to; i++, mfrom++) {
		for (UINT j = 0; j < get_col_size(); j++) {
			set(i, j, m.get(mfrom, j));
		}
	}
}


template <class T>
void MATRIX<T>::set_rows(UINT from, UINT to, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(from <= to && to < m_row_size, ("illegal matrix"));
	for (UINT i = from; i <= to; i++) {
		set_row(i, v);
	}
}


/*
cols: column need to set new value.
*/
template <class T>
void MATRIX<T>::set_row(UINT rows, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(rows < m_row_size, ("exception occur in set()"));
	for (UINT i = 0; i < m_col_size; i++) {
		set(rows, i, v);
	}
}


/*
Set all 'col' column elements of matrix
cols: column need to set new value.
*/
template <class T>
void MATRIX<T>::set_col(UINT cols, T col[], INT num)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(cols < m_col_size && num == m_row_size,
			("exception occur in set()"));
	for (UINT i = 0 ; i < m_row_size ; i++) {
		set(i, cols, col[i]);
	}
}


/*
cols: column need to set new value.
*/
template <class T>
void MATRIX<T>::set_col(UINT cols, MATRIX<T> const& v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(cols < m_col_size, ("exception occur in set()"));
	IS_TRUE(v.is_vec(), ("v must be vector"));
	IS_TRUE(v.size() == m_row_size, ("invalid vector"));
	if (v.is_rowvec()) {
		IS_TRUE(v.m_col_size == m_row_size,("unmatch vector"));
		for (UINT i = 0; i < m_row_size; i++) {
			set(i, cols, v.get(0, i));
		}
	} else if (v.is_colvec()) {
		IS_TRUE(v.m_row_size == m_row_size,("unmatch vector"));
		for (UINT i = 0; i < m_row_size; i++) {
			set(i, cols, v.get(i, 0));
		}
	} else {
		IS_TRUE(0, ("invalid operation"));
	}
}


/*
Overlays on elements.
*/
template <class T>
void MATRIX<T>::set_cols(UINT from, UINT to, MATRIX<T> const& m, UINT mfrom)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m.m_row_size == m_row_size &&
			from <= to && to < m_col_size && mfrom + to - from < m.m_col_size,
			("illegal matrix"));
	for (UINT i = 0; i < get_row_size(); i++) {
		UINT k = 0;
		for (UINT j = from; j <= to; j++, k++) {
			set(i, j, m.get(i, mfrom + k));
		}
	}
}


template <class T>
void MATRIX<T>::set_cols(UINT from, UINT to, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(from <= to && to < m_col_size, ("illegal matrix"));
	for (UINT j = from; j <= to; j++) {
		set_col(j, v);
	}
}


/*
cols: column need to set new value.
*/
template <class T>
void MATRIX<T>::set_col(UINT cols, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(cols < m_col_size, ("exception occur in set()"));
	for (UINT i = 0; i < m_row_size; i++) {
		set(i, cols, v);
	}
}


/*
Interchange row
*/
template <class T>
void MATRIX<T>::interch_row(UINT row1, UINT row2)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(row1 < m_row_size && row2 < m_row_size,
			("exception occur in interch_row()"));
	if (row1 == row2) return;
	for (UINT i = 0 ; i < m_col_size ; i++) {
		T tmp = get(row1, i);
		set(row1, i, get(row2, i));//Action: m_mat[row1][i] = m_mat[row2][i];
		set(row2, i, tmp);//Action: m_mat[row2][i] = tmp;
	}
}


/*
Interchange column
*/
template <class T>
void MATRIX<T>::interch_col(UINT col1, UINT col2)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(col1 < m_row_size && col2 < m_row_size,
			("exception occur in interch_col()"));
	if (col1 == col2) return;
	for (UINT i = 0 ; i < m_row_size ; i++) {
		T tmp = get(i,col1);
		set(i,col1, get(i,col2));//Action: m_mat[i][col1] = m_mat[i][col2];
		set(i, col2, tmp);//Action:  m_mat[i][col2] = tmp;
	}
}


template <class T>
T * MATRIX<T>::get_matrix() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	return m_mat;
}


/*
Transposing
*/
template <class T>
void MATRIX<T>::trans()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	UINT i;
	if (m_row_size == 0) return;
	IS_TRUE(m_mat,("exception occur tranpose()"));
	T * tmp_mat = (T*)::malloc(m_row_size * m_col_size * sizeof(T));
	for (i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			//Action: tmp_mat[j][i] = m_mat[i][j];
			*(tmp_mat + j*m_row_size +i) = get(i,j);
		}
	}
	::free(m_mat);
	m_mat = tmp_mat;
	i = m_row_size;
	m_row_size = m_col_size;
	m_col_size = i;
}


/*
Duplication
*/
template <class T>
void MATRIX<T>::copy(IN MATRIX<T> const& m)
{
	IS_TRUE(m_is_init && m.m_is_init, ("not yet initialize."));
	if (this == &m) return;
	if (m.size() == 0) {
		clean();
	} else {
		//m(a,b), m(x,y): a*b == x*y
		if (size() != m.size()) {
			if (m_mat != NULL) {
				::free(m_mat);
				m_mat = NULL;
			}
			m_mat = (T*)::malloc(m.size() * sizeof(T));
		}
		memcpy(m_mat, m.m_mat, m.size() * sizeof(T));
		m_row_size = m.m_row_size;
		m_col_size = m.m_col_size;
	}
	m_inhr = m.m_inhr;
}


template <class T>
bool MATRIX<T>::is_nonsig() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (!is_quad()) return false;
	if (_equal(det(), T(0))) return false;
	return true;
}


/*
Is a quad matrix?
*/
template <class T>
bool MATRIX<T>::is_quad() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0,
			("exception occur in is_quad()"));
	return m_row_size == m_col_size;
}


template <class T>
bool MATRIX<T>::is_unitary() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (!is_quad()) return false;
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			if (i == j) {
				if (get(i,j) != 1) {
					return false;
				}
			} else if (!_equal(get(i,j), T(0))) {
				return false;
			}
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::is_symm() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (!is_quad()) return false;
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			if (i == j) {
				continue;
			}
			if (!_equal(get(i,j), get(j,i))) {
				return false;
			}
		}
	}
	return true;
}


/*
NOTICE:
	'this' uses row convention
*/
template <class T>
bool MATRIX<T>::is_orth() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	MATRIX<T> tmp = *this;
	tmp.trans();
	tmp = *this * tmp;
	for (UINT i = 0; i < tmp.m_row_size; i++) {
		for (UINT j = 0; j < tmp.m_col_size; j++) {
			if (i != j && !_equal(tmp.get(i,j), T(0))) {
				return false;
			}
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::operator == (MATRIX<T> const& m) const
{
	IS_TRUE(m_is_init && m.m_is_init, ("not yet initialize."));
	if (this == &m) return true;
	if (!is_homo(m)) {
		return false;
	}
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			if (get(i,j) != m.get(i,j)) {//May be need to change precision.
				return false;
			}
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::operator != (MATRIX<T> const& m) const
{
	return !(*this == m);
}


/*
Return true if 'this' and m are homogeneous.
*/
template <class T>
bool MATRIX<T>::is_homo(MATRIX<T> const& m) const
{
	IS_TRUE(m_is_init && m.m_is_init, ("not yet initialize."));
	if (m_row_size == m.m_row_size && m_col_size == m.m_col_size) {
		return true;
	}
	return false;
}


template <class T>
void MATRIX<T>::neg()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0, ("invalid matrix"));
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			set(i, j, -get(i, j));
		}
	}
}


//Compute negaitve value of elements from row 'from' to 'to'.
template <class T>
void MATRIX<T>::neg_of_rows(UINT from, UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && to < m_row_size && from <= to,
			("invalid matrix or parameter"));
	for (UINT i = from; i <= to; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			set(i, j, -get(i, j));
		}
	}
}


//Compute negaitve value of elements from column 'from' to 'to'.
template <class T>
void MATRIX<T>::neg_of_cols(UINT from, UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && to < m_col_size && from <= to,
			("invalid matrix or parameter"));
	for (UINT j = from; j <= to; j++) {
		for (UINT i = 0; i < m_row_size; i++) {
			set(i, j, -get(i, j));
		}
	}
}


template <class T>
void MATRIX<T>::mul(T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0, ("invalid matrix"));
	if (_equal(v, T(0))) {
		zero();
		return;
	}
	if (v == 1) {
		return;
	}
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			set(i, j, (get(i, j) * v));
		}
	}
	return;
}


//Multiply times v for each element of row 'row'
template <class T>
void MATRIX<T>::mul_of_row(UINT row, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && row < m_row_size,
			("invalid matrix or parameter"));
	if (v == 1) {
		return;
	}
	if (_equal(v, T(0))) {
		set_row(row, T(0));
		return;
	}
	for (UINT j = 0; j < m_col_size; j++) {
		set(row, j, (get(row, j) * v));
	}
}


//Multiply times v for each element of row from 'from' to 'to'.
template <class T>
void MATRIX<T>::mul_of_rows(UINT from, UINT to, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && to < m_row_size && from <= to,
			("invalid matrix or parameter"));
	if (v == 1) {
		return;
	}
	if (_equal(v, T(0))) {
		set_rows(from, to, T(0));
		return;
	}
	for (UINT i = from; i <= to; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			set(i, j, (get(i, j) * v));
		}
	}
}


//Multiply times v for each element of column 'col'
template <class T>
void MATRIX<T>::mul_of_col(UINT col, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && col < m_col_size,
					("invalid matrix or parameter"));
	if (v == 1) {
		return;
	}
	if (_equal(v, T(0))) {
		set_col(col, T(0));
		return;
	}
	for (UINT i = 0; i < m_row_size; i++) {
		set(i, col, (get(i, col) * v));
	}
}


//Multiply times v for each element of column from 'from' to 'to'.
template <class T>
void MATRIX<T>:: mul_of_cols(UINT from, UINT to, T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && to < m_col_size && from <= to,
			("invalid matrix or parameter"));
	if (v == 1) {
		return;
	}
	if (_equal(v, T(0))) {
		set_cols(from, to, T(0));
		return;
	}
	for (UINT j = from; j <= to; j++) {
		for (UINT i = 0; i < m_row_size; i++) {
			set(i, j, (get(i, j) * v));
		}
	}
}


//Add one row to another row. Row 'to' changed.
template <class T>
void MATRIX<T>::add_row_to_row(UINT from, UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && from < m_row_size &&
			to < m_row_size, ("invalid matrix or parameter"));
	for (UINT j = 0; j < m_col_size; j++) {
		set(to, j, (get(from, j) + get(to, j)));
	}
}


//Add one row of 'm' to one row of 'this'. Row 'to' changed.
template <class T>
void MATRIX<T>::add_row_to_row(IN MATRIX<T> const& m,
							IN UINT mfrom, IN UINT to)
{
	IS_TRUE(m_is_init && m.m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && mfrom < m.m_row_size &&
			to < m_row_size, ("invalid matrix or parameter"));
	IS_TRUE(m_col_size == m.m_col_size, ("unmatch row size"));
	for (UINT j = 0; j < m_col_size; j++) {
		set(to, j, (m.get(mfrom, j) + get(to, j)));
	}
}


//Add one column to another column. Column 'to' changed.
template <class T>
void MATRIX<T>::add_col_to_col(UINT from, UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && from < m_col_size &&
			to < m_col_size, ("invalid matrix or parameter"));
	for (UINT i = 0; i < m_row_size; i++) {
		set(i, to, (get(i, from) + get(i, to)));
	}
}


//Add one column of 'm' to one column of 'this'. Column 'to' changed.
template <class T>
void MATRIX<T>::add_col_to_col(IN MATRIX<T> const& m,
							   IN UINT mfrom, IN UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && mfrom < m.m_col_size &&
			to < m_col_size, ("invalid matrix or parameter"));
	IS_TRUE(m_row_size == m.m_row_size, ("unmatch col size"));
	for (UINT i = 0; i < m_row_size; i++) {
		set(i, to, (m.get(i, mfrom) + get(i, to)));
	}
}


//Mutiply times v to element of row 'from', and add such element to row 'to'.
template <class T>
void MATRIX<T>::mul_and_add_row(UINT from, T v, UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && from < m_row_size &&
			to < m_row_size, ("invalid matrix or parameter"));
	for (UINT j = 0; j < m_col_size; j++) {
		set(to, j, ((get(from, j) * v) + get(to, j)));
	}
}


template <class T>
void MATRIX<T>::mul_and_add_col(UINT from, T v, UINT to)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0 && from < m_col_size &&
			to < m_col_size, ("invalid matrix or parameter"));
	for (UINT i = 0; i < m_row_size; i++) {
		set(i, to, ((get(i, from) * v) + get(i, to)));
	}
}


template <class T>
UINT MATRIX<T>::get_row_size() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	return m_row_size;
}


template <class T>
UINT MATRIX<T>::get_col_size() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	return m_col_size;
}


/*
Set all elements with value 'v'
*/
template <class T>
void MATRIX<T>::set_all(T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	for (UINT j = 0; j < m_row_size; j++) {
		for (UINT i = 0; i < m_col_size; i++) {
			set(j, i, v);
		}
	}
}


template <class T>
INT MATRIX<T>::_reverse_order_number(INT * numbuf, UINT numlen)
{
	INT ron = 0;
	for (UINT i = 0; i < numlen - 1; i++) {
		for (UINT j = i+1; j < numlen; j++) {
			if (numbuf[i] > numbuf[j]) {
				ron++;
			}
		}
	}
	return ron;
}


/*v: current digital need choose one slot in buf.
 *posbuf: slot buf
  posbufnum: number of slot
  n: natural number
 * */
template <class T>
void MATRIX<T>::_full_permutation_recur(INT v, INT * posbuf,
										UINT posbufnum, INT n, T & det)
{
	for (UINT i = 0; i < posbufnum; i++) {
		if (posbuf[i] == 0) { //slot of position 'i' in 'posbuf' is avaible.
			posbuf[i] = v;
			if (v == n) {
				/*ONLY for debugging
				printf("\n full_perm_order: ");
				for (INT w = 0; w < posbufnum; w++) {
					printf("%d,",posbuf[w]);
				}
				printf("\telem: ");
				for (INT w = 0; w < posbufnum; w++) {
					T gg = get(w, (posbuf[w]-1));
					printf("%d,",gg.num());
				}
				printf(" : ron=%d\n",
						_reverse_order_number(posbuf, posbufnum));
				*/
				T t;
				t = 1;
				for (UINT j = 0; j < posbufnum; j++) {
					IS_TRUE(posbuf[j]>=1, ("Invalid subscript"));
					t = t * get(j, (posbuf[j]-1));
				}
				INT ron = _reverse_order_number(posbuf, posbufnum);
				if ((ron % 2) != 0)	{
					T minus;
					minus = -1;
					t = t * minus;
				}
				det = det + t;
				posbuf[i] = 0;
				return;
			} else {
				_full_permutation_recur(v+1, posbuf, posbufnum, n, det);
				posbuf[i] = 0;
			}
		}
	}
}


//Laplace extension
template <class T>
T MATRIX<T>::_full_permutation(UINT n)
{
	IS_TRUE(n > 1, ("Invalid number"));
	INT * posbuf = (INT*)::malloc(sizeof(INT) * n);
	T det;
	det = 0;
	memset(posbuf, 0, sizeof(INT) * n);
	for (UINT i = 0; i < n; i++) {
		posbuf[i] = 1;
		_full_permutation_recur(2, posbuf, n, n, det);
		posbuf[i] = 0;
	}
	::free(posbuf);
	return det;
}


/*
Calculate determinant result, only consider square matrix.
Fast method:
	det(A) = (-1)^r * det(U),r is the number of times of interchanging.
	U is upper triangular matrix.
*/
template <class T>
T MATRIX<T>::det() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size >= 1 && m_col_size >= 1, ("strange matrix"));
	if (!is_quad()) {
		return T(0);
	}
	T detv;
	switch (m_row_size) {
	case 1:
		return get(0,0);
	case 2:
		{
			detv = get(0,0) * get(1,1) - get(0,1) * get(1,0);
			return detv;
		}
	case 3:
		{
			if (is_up_triangular() || is_low_triangular()) {
				detv = get(0,0)*get(1,1)*get(2,2);
			} else if (is_anti_up_triangular() || is_anti_low_triangular()) {
				T sign = ((m_row_size * (m_row_size - 1))/2) % 2 == 0 ?
						 T(1) : T(-1);
				detv = get(2,0)*get(1,1)*get(0,2)*sign;
			} else {
				detv = get(0,0) * get(1,1) * get(2,2) +
					   get(1,0) * get(2,1) * get(0,2) +
					   get(0,1) * get(1,2) * get(2,0) -
					   get(0,2) * get(1,1) * get(2,0) -
					   get(0,1) * get(1,0) * get(2,2) -
					   get(2,1) * get(1,2) * get(0,0);
			}
			return detv;
		}
	default:
		{
			if (is_up_triangular() || is_low_triangular()) {
				detv = 1;
				for (UINT i = 0; i < m_row_size; i++) {
					detv = detv * get(i,i);
				}
			} else if (is_anti_up_triangular() || is_anti_low_triangular()) {
				T sign =
					((m_row_size * (m_row_size - 1))/2) % 2 == 0 ?
					 T(1) : T(-1);
				detv = 1;
				for (UINT i = 0; i < m_row_size; i++) {
					detv = detv * get(i,m_row_size - 1 - i);
				}
			} else {
//#define DET_M1 //M1 is slow slightly
#ifdef DET_M1
				detv = _full_permutation(m_row_size);
#else
				MATRIX<T> a = *this;
				UINT r = 0, j;
				for (j = 0; j < a.m_col_size; j++) {
					//Finding the perfect pivot entry or permuting of
					//non-zero pivot entry.
					INT swap_row = -1;
					T swap_entry;
					for (UINT k = j; k < a.m_row_size; k++) {
						T tmp = a.get(k,j);
						if (_equal(tmp, T(0))) { //zero entry
							continue;
						}

						if (swap_row == -1) {
							//The first non-zero entry is the candidate,
							swap_row = k;
							swap_entry = tmp;
							if (swap_entry == 1) {
								break;
							}
						} else if (tmp == 1) {
							//but we are glad to find the row with
							//unitary entry as the pivot,
							swap_row = k;
							break;
						} else if (abs(swap_entry) < abs(tmp)) {
							//or always find the maximum entry as pivot entry.
							swap_row = k;
							swap_entry = tmp;
						}
					}//end for

					if (swap_row == -1) {
						//Not find nonzero element.
						//There are more than 2 of zeros in one column.
						//And matrix is singular.
						detv = 0;
						goto FIN;
					}

					if (swap_row != (INT)j) { //Ensure diagonal nonzero.
						a.interch_row(swap_row, j);
						r++;
					}

					//Reduce other column entries below pivot entry to zero.
					for (UINT i = j+1; i < a.m_row_size; i++) {
						if (!_equal(a.get(i,j), T(0))) {
							T tmp = - (a.get(i,j) / a.get(j,j));
							a.mul_and_add_row(j, tmp, i);
						}
					}
				}//end for

				detv = 1;
				for (j = 0; j < a.m_col_size; j++) {
					detv = detv * a.get(j,j);
				}
				if (ODD(r)) {
					detv = -(detv);
				}
#endif
			}
		}
	}
FIN:
	return detv;
}


//Allow operation as 'x=y=z'
template <class T>
MATRIX<T> & MATRIX<T>::operator = (MATRIX<T> const& m)
{
	IS_TRUE(m_is_init && m.m_is_init, ("not yet initialize."));
	copy(m);
	return *this;
}


/*
Return true if matrix is nonsingular, otherwise return false.

'e': inverted matrix.

NOTICE:
	'this' must use row convention.
*/
template <class T>
bool MATRIX<T>::inv(OUT MATRIX<T> & e)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size >= 1 && m_col_size >= 1, ("strange matrix"));
	if (!is_quad()) {
		e.clean();
		return false;
	}
	MATRIX<T> p(m_row_size, m_col_size, &m_inhr);
	p = *this;
	if (this == &e) {
		e.zero();
	} else {
		e.reinit(m_row_size, m_col_size, &m_inhr); //unitary matrix
	}
	switch (m_row_size) {
	case 1:
		IS_TRUE0(!_equal(p.get(0,0), T(0)));
		e.set(0,0, 1/p.get(0,0));
		return true; //non-singular
	case 2:
		{
			//1/(ad-bc)(d -b)
			//         (-c a)
			T k = p.get(0,0) * p.get(1,1) - p.get(0,1) * p.get(1,0);
			if (_equal(k, T(0))) {
				e.clean();
				return false; //singular
			}
			k = 1 / k;
			e.set(0,0, p.get(1,1));
			e.set(1,1, p.get(0,0));
			e.set(0,1, -1 * p.get(0,1));
			e.set(1,0, -1 * p.get(1,0));
			e.mul(k);
			return true; //non-singular
		}
	default:
		{
			e.eye(1);
			bool is_nonsingular = false;
			for (UINT j = 0; j < p.m_col_size; j++) {
				//Finding the perfect pivot entry or permuting of
				//non-zero pivot entry.
				INT swap_row = -1;
				T swap_entry;
				for (UINT k = j; k < p.m_row_size; k++) {
					T tmp = p.get(k,j);
					if (_equal(tmp, T(0))) { //zero entry
						continue;
					}
					if (swap_row == -1) {
						//The first non-zero entry is the candidate,
						swap_row = k;
						swap_entry = tmp;
						if (swap_entry == 1) {
							break;
						}
					} else if (tmp == 1) {
						swap_row = k; //but we are glad to find the row with
								      //unitary entry as the pivot,
						break;
					} else if (abs(swap_entry) < abs(tmp)) {
						//or always find the maximum entry as pivot entry.
						swap_row = k;
						swap_entry = tmp;
					}
				}
				if (swap_row == -1) {
					is_nonsingular = false;
					e.clean();
					goto FIN;
				}
				if (swap_row != (INT)j) {
					p.interch_row(swap_row, j);
					e.interch_row(swap_row, j);
				}

				//Reduce diagonal entry to unitary
				if (p.get(j,j) != 1) {
					T tmp = 1 / p.get(j,j);
					p.mul_of_row(j, tmp);
					e.mul_of_row(j, tmp);
				}

				//Reduce other column entries below pivot entry to zero.
				for (UINT i = 0; i < p.m_row_size; i++) {
					if (i != j) {
						if (!_equal(p.get(i,j), T(0))) {
							T tmp = -1 * p.get(i,j);
							p.mul_and_add_row(j, tmp, i);
							e.mul_and_add_row(j, tmp, i);
						}
					}
				}
			}
			is_nonsingular = true;
FIN:
			return is_nonsingular;
		}
	}
	IS_TRUE(0, ("can not get here"));
	return false; //to avoid compiling warning.
}


/*
Full rank-decomposition

'f': m*r full rank matrix
'g': r*n full rank matrix
Let A is m*n matrix, rank is r, and A,f,g satisfied A=f*g.
*/
template <class T>
void MATRIX<T>::frd(OUT MATRIX<T> & f, OUT MATRIX<T> & g)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 1 && m_col_size > 1, ("strange matrix"));
	MATRIX<T> tmp;
	UINT rankr = rank(&tmp, true);
	g.grow_row(tmp, 0, rankr - 1);
	UINT rank_row = 0;
	for (UINT j = 0; j < tmp.m_col_size; j++) {
		if (_equal(tmp.get(rank_row, j), T(1))) {
			f.grow_col(*this, j, j);
			rank_row++;
			if (rank_row == rankr) {
				break;
			}
		}
	}
}


/*
Computation of left-inverse of column full rank matrix.

'x': left-inverse

That is satisfied:
	XA = I
*/
template <class T>
bool MATRIX<T>::linv(OUT MATRIX<T> & x)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 1 && m_col_size > 1, ("strange matrix"));
	IS_TRUE(m_row_size >= m_col_size, ("unexpected matrix"));
	x.clean();
	MATRIX<T> transm = *this;
	transm.trans();
	MATRIX<T> prod = transm * *this;
	if (!prod.inv(prod)) {
		return false;
	}
	x = prod * transm;
	return true;
}


/*
Computation of right-inverse of row full rank matrix.

'x': right-inverse

That is satisfied:
	AX = I
*/
template <class T>
bool MATRIX<T>::rinv(OUT MATRIX<T> & x)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 1 && m_col_size > 1, ("strange matrix"));
	IS_TRUE(m_row_size <= m_col_size, ("unexpected matrix"));
	x.clean();
	MATRIX<T> transm = *this;
	transm.trans();
	MATRIX<T> prod = *this * transm;
	if (!prod.inv(prod)) {
		return false;
	}
	x = transm * prod;
	return true;
}


/*
Calculation of Moore-Penrose inverse matrix.
Return true if matrix is nonsingular, otherwise return false.

'x': pseudo-inverse

The Moore-Penrose is the most commonly encountered pseudoinverse
that is also known as {1,2,3,4}-inverse, or pseudo inverse.
That is satisifed:
	1.AXA = A
	2.XAX = X
	3.(AX)^H = AX
	4.(XA)^H = XA
*/
template <class T>
void MATRIX<T>::pinv(OUT MATRIX<T> & x)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	refinv(x);
}


/*
Calculation of {1,4}-inverse matrix.

'x': least-square inverse

{1,4}-inverse is also named least-square inverse, a special case
of a g-inverse.
That is satisifed:
	1.AXA = A
	4.(AX)^H = AX
*/
template <class T>
void MATRIX<T>::lstinv(OUT MATRIX<T> & x)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	refinv(x);
}


/*
Calculation of {1,3}-inverse matrix.

'x': least-norm inverse

{1,3}-inverse is also named least-norm inverse,
a special case of a g-inverse.
That is satisifed:
	1.AXA = A
	3.(XA)^H = XA
*/
template <class T>
void MATRIX<T>::minv(OUT MATRIX<T> & x)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	refinv(x);
}


/*
Calculation of {1,2}-inverse matrix.
Return true if matrix is nonsingular, otherwise return false.

'x': reflexive inverse

{1,2}-inverse is also named reflexive g-inverse,
a special case of a g-inverse.
That is satisifed:
	1.AXA = A
	2.XAX = X
*/
template <class T>
void MATRIX<T>::refinv(OUT MATRIX<T> & x)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 1 && m_col_size > 1, ("strange matrix"));
	MATRIX<T> f(0, 0, &m_inhr), g(0, 0, &m_inhr);
	frd(f,g);
	MATRIX<T> fl(0, 0, &m_inhr), gr(0, 0, &m_inhr);
	f.linv(fl);
	g.rinv(gr);
	x = gr * fl;
}


/*
Calculation of {1}-inverse matrix.
Return true if matrix is nonsingular, otherwise return false.

Matrix {1}-inverse is a special case of a general type of pseudoinverse
known as g-inverse.
That is satisfied:
	1.AXA = A
*/
template <class T>
bool MATRIX<T>::ginv(OUT MATRIX<T> & x)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 1 && m_col_size > 1, ("strange matrix"));
	if (rank() == MIN(m_row_size, m_col_size)) {
		if (m_row_size == m_col_size) {
			//Nonsingular matrix inverse
			return inv(x);
		}

		//Computing Moore-Penrose inverse.
		MATRIX<T> trans_m = *this;
		trans_m.trans();

		MATRIX<T> * A = NULL,* At = NULL;
		if (m_row_size < m_col_size) {
			At = this;
			A = &trans_m;
		} else {
			A = this;
			At = &trans_m;
		}

		//Ginv = ((At*A)^-1)*At
		MATRIX<T> quad = *At * *A;
		bool res = quad.inv(quad);
		IS_TRUE(res, ("quad should be invertible!"));
		x = quad * *At;
		if (m_row_size < m_col_size) {
			x.trans();
		}

		//test
		MATRIX<T> t = *this * x;
		t = t**this ;
		//
		return false;
	}

	/*
	Computing matrix {1}-inverse
	Generate P,Q, that makes PAQ=J, where J is [I 0],
											   [0 0]
	and Ag is Q[I X]P.
			   [Y Z]
	*/
	MATRIX<T> P(m_row_size, m_row_size, &m_inhr);
	MATRIX<T> Q(m_col_size, m_col_size, &m_inhr);
	P.eye(1);
	Q.eye(1);

	MATRIX<T> tmpthis = *this;
	UINT rankr = 0, row, col;

	/*
	Generating nonsingular matrix P, that obtained by performing the
	same operations which to reduce 'A' to the matrix with (rank*rank)
	identity sub-matrix on left-corner on the identity matrix.
	*/

	//Generating P by row elementary operations.
	for (row = 0, col = 0;
		row < tmpthis.m_row_size && col < tmpthis.m_col_size;
		row++, col++) {
		//Finding the perfect pivot entry or permuting of non-zero pivot entry.
		INT swap_row = -1;
		T swap_entry;

		//Find  column with non-zero entry. 'w' meas 'which'
		for (UINT w = col; w < tmpthis.m_col_size; w++) {
			for (UINT k = row; k < tmpthis.m_row_size; k++) {
				T tmp = tmpthis.get(k, w);
				if (_equal(tmp, T(0))) {
					continue;
				}
				if (swap_row == -1) {
					//The first non-zero entry is the candidate
					swap_row = k;
					swap_entry = tmp;
					if (swap_entry == 1) {
						break;
					}
				} else if (tmp == 1) {
					//We are preferable to interchange the row
					//with unitary entry as the pivot
					swap_row = k;

					swap_entry = tmp;
					break;
				} else if (abs(swap_entry) < abs(tmp)) {
					//Find the maximum entry as pivot entry.
					swap_row = k;
					swap_entry = tmp;
				}
			}//end for
			if (swap_row == -1) {
				continue; //Try next column, trapezoid matrix
			}
			if (swap_row != (INT)row) {
				tmpthis.interch_row(swap_row, row);
				P.interch_row(swap_row, row);
			}
			col = w;
			break;
		}//end for

		if (swap_row == -1) {
			goto FIN; //All elements of row 'row' are zero.
		}

		//Reducing pivot entry (row, col) to unitary.
		if (tmpthis.get(row, col) != 1) {
			//Notice, quotient may be zero if den less than 1.
			T tmp = 1 / (tmpthis.get(row, col));
			tmpthis.mul_of_row(row, tmp);
			P.mul_of_row(row, tmp);
		}

		//Reducing other column-wise entries except for the pivot entry
		//to zero.
		UINT strow = 0;
		for (UINT i = strow; i < tmpthis.m_row_size; i++) {
			if (i != row) {
				//For reducing the computation cost.
				if (!_equal(tmpthis.get(i, col), T(0))) {
					//T tmp = -1 * ptr->get(i, col);
					T tmp = -(tmpthis.get(i, col)) / tmpthis.get(row, col);
					tmpthis.mul_and_add_row(row, tmp, i);
					P.mul_and_add_row(row, tmp, i);
				}
			}
		}//end for
		rankr++;
		tmpthis.adjust();
	}//end for (INT row = 0...

FIN:
	///TEST
	//MATRIX<T> t = P**this;
	///

	//Generating Q by column elementary operations
	for (row = 0; row < rankr; row++) {
		if (!_equal(tmpthis.get(row, row), T(1))) {
			/*
			It is necessary to interchange second column with third.
			CASE: 1 0 0 *
			      0 0 1 *
				=>
			      1 0 0 *
			      0 1 0 *
			*/
			bool find = false;
			for (UINT col = row+1; col < tmpthis.m_col_size; col++) {
				if (_equal(tmpthis.get(row, col), T(1))) {
					tmpthis.interch_col(row, col);
					Q.interch_col(row, col);
					find = true;
					break;
				}
			}
			IS_TRUE(find, ("exception occurring!"));
		}
		for (UINT col = row+1; col < tmpthis.m_col_size; col++) {
			if (!_equal(tmpthis.get(row, col), T(0))) {
				T tmp = -1 * tmpthis.get(row, col);
				tmpthis.mul_and_add_col(row, tmp, col);
				Q.mul_and_add_col(row, tmp, col);
			}
		}//end for
	}//end for

	///TEST
	//t = P**this*Q;
	///

	MATRIX<T> I(rankr, rankr, &m_inhr);
	I.eye(1);
	I.grow_all(Q.m_col_size - rankr, P.m_row_size - rankr);
	x = Q*I*P;


	///test
	//t = *this * x * *this;
	///
	return false;
}


//Initialize diagonal entry to 'v'
template <class T>
void MATRIX<T>::setdiag(IN SVECTOR<T> const& v, UINT vlen)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(),("must be square"));
	IS_TRUE(vlen == m_row_size,("unmatch vector"));
	for (UINT i = 0; i < m_row_size; i++) {
		set(i, i, v[i]);
	}
}


//Set 'v' to row vector by diagonal entry.
template <class T>
void MATRIX<T>::getdiag(OUT MATRIX<T> & v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0, ("matrix is empty."));
	UINT b = MIN(m_row_size, m_col_size);
	v.reinit(1, b);
	for (UINT i = 0; i < b; i++) {
		v.set(0, i, get(i, i));
	}
}


/*
Initialize diagonal entry of vector v

NOTICE:
	'v' is vector
*/
template <class T>
void MATRIX<T>::setdiag(IN MATRIX<T> const& v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(), ("must be square"));
	IS_TRUE(v.is_vec(), ("must be vector"));
	IS_TRUE(v.size() == m_row_size, ("out of boundary"));
	if (v.is_rowvec()) {
		for (UINT i = 0; i < m_row_size; i++) {
			set(i, i, v.get(0,i));
		}
	} else {
		for (UINT i = 0; i < m_row_size; i++) {
			set(i, i, v.get(i,0));
		}
	}
}


template <class T>
void MATRIX<T>::eye(T v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(),("identity matrix must be square"));
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			if (i == j) {
				set(i, j, v);
			} else {
				set(i, j, 0);
			}
		}
	}
}


template <class T>
void MATRIX<T>::zero()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			set(i, j, 0);
		}
	}
}


template <class T>
void MATRIX<T>::fzero()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (m_mat != NULL) {
		memset(m_mat, 0, sizeof(T) * m_row_size * m_col_size);
	}
}


template <class T>
bool MATRIX<T>::is_low_triangular() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(),("must be square"));
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = i + 1; j < m_col_size; j++) {
			if (!_equal(get(i,j), T(0))) {
				return false;
			}
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::is_anti_up_triangular() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(),("must be square"));
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size - 1 - i; j++) {
			if (!_equal(get(i,j), T(0))) {
				return false;
			}
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::is_rowvec() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	return m_row_size == 1;
}


template <class T>
bool MATRIX<T>::is_colvec() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	return m_col_size == 1;
}


template <class T>
bool MATRIX<T>::is_vec() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	return is_rowvec() || is_colvec();
}


template <class T>
bool MATRIX<T>::is_rowequ(UINT row, T v) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(row < m_row_size, ("out of boundary"));
	for (UINT i = 0;  i < m_col_size; i++) {
		if (!_equal(get(row, i), v)) {
			return false;
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::is_rowequ(UINT row, IN MATRIX<T> const& m, IN UINT mrow) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(row < m_row_size &&
			mrow < m.m_row_size &&
			m_col_size == m.m_col_size, ("out of boundary"));
	for (UINT i = 0;  i < m_col_size; i++) {
		if (!_equal(get(row, i), m.get(mrow, i))) {
			return false;
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::is_colequ(UINT col, T v) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(col < m_col_size, ("out of boundary"));
	for (UINT i = 0;  i < m_row_size; i++) {
		if (!_equal(get(i, col), v)) {
			return false;
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::is_colequ(UINT col, IN MATRIX<T> const& m, IN UINT mcol) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(col < m_col_size &&
			mcol < m.m_col_size &&
			m_row_size == m.m_row_size, ("out of boundary"));
	for (UINT i = 0;  i < m_row_size; i++) {
		if (!_equal(get(i, col), m.get(i, mcol))) {
			return false;
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::is_up_triangular() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(),("must be square"));
	for (UINT j = 0; j < m_col_size; j++) {
		for (UINT i = j + 1; i < m_row_size; i++) {
			if (!_equal(get(i,j), T(0))) {
				return false;
			}
		}
	}
	return true;
}


template <class T>
bool MATRIX<T>::is_anti_low_triangular() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(),("must be square"));
	for (UINT j = 0; j < m_col_size; j++) {
		for (UINT i = m_row_size - 1; i > m_row_size - 1 - j; i--) {
			if (!_equal(get(i,j), T(0))) {
				return false;
			}
		}
	}
	return true;
}


/*
Reduce matrix to row-echelon normal form, that the pivot diagonal element
to 1, and other elements in column are zero.
*/
template <class T>
void MATRIX<T>::eche()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	MATRIX<T> tmp;
	rank(&tmp);
	copy(tmp);
}


/*
Calculate Basis of rows vector.

'b': use row convention. Each row indicate one basis

NOTICE:
	'this' uses row convention.
*/
template <class T>
void MATRIX<T>::basis(OUT MATRIX<T> & b)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	UINT r = rank(&b, false);
	if (r < b.m_row_size) { //b is not full rank.
		MATRIX<T> tmp;
		b.inner(tmp, 0, 0, r-1, b.m_col_size-1);
		b = tmp;
	}
}


/*
Calculate Algebraic Complement of element A(row,col).
*/
template <class T>
void MATRIX<T>::algc(UINT row, UINT col, OUT MATRIX<T> & m)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(row < m_row_size && col < m_col_size, ("not an entry"));
	m.reinit(m_row_size - 1, m_col_size - 1, &m_inhr);
	for (UINT i = 0; i < m_row_size; i++) {
		for (UINT j = 0; j < m_col_size; j++) {
			if (i == row || j == col) {
				continue;
			}
			UINT ni = i, nj = j;
			if (ni > row) {
				ni--;
			}
			if (nj > col) {
				nj--;
			}
			m.set(ni, nj, get(i,j));
		}
	}
}


/*
Calculate classical Adjoint Matrix.
*/
template <class T>
bool MATRIX<T>::adj(OUT MATRIX<T> & m)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(), ("adj for rectangle matrix"));
//#define BY_DEFINITION //Use the your favorite method.
#ifdef BY_DEFINITION
	m.reinit(m_row_size, m_col_size, &m_inhr);
	switch (m_row_size) {
	case 2:
		{
			m.set(0,0,get(1,1));
			m.set(0,1,-get(0,1));
			m.set(1,0,-get(1,0));
			m.set(1,1,get(0,0));
		}
		break;
	default:
		{
			MATRIX<T> tmp;
			for (UINT i = 0; i < m_row_size; i++) {
				for (UINT j = 0; j < m_col_size; j++) {
					algc(i,j,tmp);
					T c = ODD(i+j)?-1:1;
					m.set(i, j, c * tmp.det());
				}
			}
			m.trans();
		}
	}
#else //BY INVERSING MATRIX
	T dv = this->det();
	if (_equal(dv, T(0))) {
		return false;
	}
	inv(m);
	m.mul(dv);
#endif
	return true;
}


/*
Compute Null Space.
Null Space of A as the general result of formular of Ax=0.

Null space is represented implicitly by matrix A.
The first step reduces augmented matrix [A 0] to echelon form.
Since augmented vector be always zero, we ignored that during
each step.

'ns': basis set of null space, shown as column convention,
	nonzero columns is the basis of null space of A.

	e.g: Given vector x is [x1,x2,x3], 'ns' turned out to be:
			1  0  0
			0  0 -1
			0  0  1

		After reduced augmented matrix [A 0], the general set is:
			x1 = x1 + 0 +  0
			x2 = 0  + 0 + -x3
			x3 = 0  + 0 +  x3
		and the solution form is:
			[x1,x2,x3] = a*[1,0,0] + b*[0,-1,1]
			where [1,0,0], [0,-1,1] are basis of null space, a, b
			are free variables.
		That is any combinations of basis of null space are pertain to
		null space of 'this'.

NOTICE:
	'this' uses row convention.
 */
template <class T>
void MATRIX<T>::null(OUT MATRIX<T> & ns)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	ns.reinit(m_col_size, m_col_size, &m_inhr);
	MATRIX<T> tmp;

	/*
	Reduce matrix to echelon form.
	This operation usually generates the general form solution of Ax=0.
	Since augmented vector be always zero, we ignored last column during
	each step.
	*/
	this->rank(&tmp, true);

	/*
	Format the generating solution to combination of vectors, and
	using free(unconstrained) variables as the weight of vector.
	e.g:
		The combination of vectors:
			[x1,x2,x3,x4] =
					x1*[0,0,0,0] + x2*[2,1,0,0] + x3*[0,0,0,0] + x4*[1,0,-2,1]
	*/
	ns.eye(1);
	INT unum = tmp.m_col_size; //Number of unknown.
	for (UINT nsrow = 0, row = 0; row < tmp.m_row_size; row++) {
		//Find non-zero leading entry.
		UINT col;
		bool find_non_zero = false;
		for (col = row; col < tmp.m_col_size; col++) {
			if (!_equal(tmp.get(row, col), T(0))) {
				nsrow = col;
				find_non_zero = true;
				break;
			}
		}
		if (!find_non_zero) {
			break;
		}
		ns.set(nsrow, col, 0);
		for (UINT k = col + 1; k < tmp.m_col_size; k++) {
			ns.set(nsrow, k, -(tmp.get(row, k)));
		}
	}
}


template <class T>
T MATRIX<T>::tr()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(), ("must be quad"));
	T trv = 0;
	for (UINT i = 0; i < m_row_size; i++) {
		trv = trv + get(i,i);
	}
	return trv;
}


/*
Compute rank of 'this', and the corresponded linear indepent vectors store
as row convention.

'basis': output the matrix that is transformed in
	upper trapezoid matix as a result of elementary row transformation.
'is_unitary': True indicates the routine will reduce the pivot element 'pv'
	to 1, and eliminating all other same column elements to zero, otherwise
	do nothing of those operations.

NOTICE:
	In actually the estimation of rank is not a simply problem.
		See details in "Numerical Linear Algebra and Optimization" ,
		Philip E. Gill.
*/
template <class T>
UINT MATRIX<T>::rank(OUT MATRIX<T> * basis, bool is_unitarize)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 0 && m_col_size > 0, ("strange matrix"));
	UINT rankv = 0;
	MATRIX<T> * ptr = NULL;
	MATRIX<INT> * rowpos = NULL; //record change process of rows.
	if (basis != NULL) {
		ptr = basis;
	} else {
		ptr = new MATRIX<T>(m_row_size, m_col_size, &m_inhr);
		is_unitarize = true;
	}
	IS_TRUE0(ptr != this);
	ptr->copy(*this);
	if (!is_unitarize) {
		rowpos = new MATRIX<INT>(ptr->get_row_size(), 1);
		for (UINT i = 0; i < rowpos->get_row_size(); i++) {
			rowpos->set(i, 0, i);
		}
	}
	for (UINT row = 0, col = 0;
		row < ptr->m_row_size && col < ptr->m_col_size;
		row++, col++) {
		//Finding the perfect pivot entry or permuting
		//of non-zero pivot entry.
		INT swap_row = -1;
		T swap_elem;

		//Find  column with non-zero entry. 'w' meas 'which'
		for (UINT w = col; w < ptr->m_col_size; w++) {
			for (UINT k = row; k < ptr->m_row_size; k++) {
				T tmp = ptr->get(k, w);
				if (_equal(tmp, T(0))) {
					continue;
				}
				if (swap_row == -1) {
					//The first non-zero element is the candidate.
					swap_row = k;
					swap_elem = tmp;
					if (swap_elem == 1) {
						break;
					}
				} else if (tmp == 1) {
					//We prefered regarding the row with
					//unitary entry as the pivot.
					swap_row = k;
					swap_elem = tmp;
					break;
				} else if (abs(swap_elem) < abs(tmp)) {
					//Find the maximum entry as pivot entry.
					swap_row = k;
					swap_elem= tmp;
				}
			} //end for
			if (swap_row == -1) {
				continue; //Try next column, trapezoid matrix
			}
			if (swap_row != (INT)row) {
				ptr->interch_row(swap_row, row);
				if (rowpos != NULL)
					rowpos->interch_row(swap_row, row);
			}
			col = w;
			break;
		} //end for

		if (swap_row == -1) {
			goto FIN; //All elements of 'row' are zero.
		}

		//Reducing pivot entry (row, col) to unitary.
		if (is_unitarize && ptr->get(row, col) != 1) {
			//Notice, quotient may be zero if den less than 1.
			T tmp = 1 / (ptr->get(row, col));
			ptr->mul_of_row(row, tmp);
		}

		//Eliminate all other elements at current column to zero, except
		//for the pivot element.
		UINT start_row = 0; //elminate all elment of column
		if (!is_unitarize) {
			start_row = row + 1;
		}
		for (UINT i = start_row; i < ptr->m_row_size; i++) {
			if (i != row) {
				if (!_equal(ptr->get(i, col), T(0))) {
					//T tmp = -1 * ptr->get(i, col);
					T tmp = -(ptr->get(i, col)) / ptr->get(row, col);
					ptr->mul_and_add_row(row, tmp, i);
				}
			}
		} //end for
		rankv++;
		ptr->adjust();
	} //end for (INT row = 0...
FIN:
	if (rowpos != NULL) {
		IS_TRUE0(ptr == basis);
		IS_TRUE0(rankv <= rowpos->get_row_size());
		if (rankv < this->get_row_size()) {
			ptr->reinit(rankv, ptr->get_col_size());
			for (UINT i = 0; i < rankv; i++) {
				ptr->set_rows(i, i, *this, rowpos->get(i, 0));
			}
		}
		delete rowpos;
	}
	if (ptr != basis) {
		delete ptr;
	}
	return rankv;
}


//Inserting one empty row before 'ridx'.
template <class T>
void MATRIX<T>::insert_row_before(UINT ridx)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(ridx < m_row_size && m_row_size > 0, ("out of bound"));
	MATRIX<T> tmp(&m_inhr);
	if (ridx == 0) {
		tmp.grow_col(m_col_size);
		tmp.grow_row(*this, 0, m_row_size - 1);
		*this = tmp;
		return;
	}
	inner(tmp, 0, 0, ridx-1, m_col_size - 1);
	tmp.grow_row(1);
	tmp.grow_row(*this, ridx, m_row_size - 1);
	*this = tmp;
}


/*
Inserting empty rows before 'ridx'.
*/
template <class T>
void MATRIX<T>::insert_rows_before(UINT ridx, UINT rnum)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(ridx < m_row_size && m_row_size > 0, ("out of bound"));
	MATRIX<T> tmp(&m_inhr);
	if (rnum == 0) {
		return;
	}
	if (ridx == 0) {
		tmp.grow_col(m_col_size);
		tmp.grow_row(rnum - 1);
		tmp.grow_row(*this, 0, m_row_size - 1);
		*this = tmp;
		return;
	}
	inner(tmp, 0, 0, ridx-1, m_col_size - 1);
	tmp.grow_row(rnum);
	tmp.grow_row(*this, ridx, m_row_size - 1);
	*this = tmp;
}


/*
Insert rows before 'ridx' which rows copy from matrix 'm'
from 'from' to 'to'.
*/
template <class T>
void MATRIX<T>::insert_rows_before(UINT ridx, IN MATRIX<T> const& m,
								   UINT mfrom, UINT mto)
{
	IS_TRUE(m_is_init && m.m_is_init, ("not yet initialize."));
	IS_TRUE(ridx < m_row_size && m_row_size > 0 &&
			mfrom < m.m_row_size && mto < m.m_row_size && mfrom <= mto,
			("out of bound"));
	MATRIX<T> tmp(&m_inhr);
	if (ridx == 0) {
		tmp.grow_col(m_col_size);
		tmp.grow_row(m, mfrom, mto);
		tmp.grow_row(*this, 0, m_row_size - 1);
		tmp.del_row(0);
		*this = tmp;
		return;
	}
	inner(tmp, 0, 0, ridx-1, m_col_size - 1);
	tmp.grow_row(m, mfrom, mto);
	tmp.grow_row(*this, ridx, m_row_size - 1);
	*this = tmp;
}


/*
Inserting one empty column before 'cidx'.
*/
template <class T>
void MATRIX<T>::insert_col_before(UINT cidx)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(cidx < m_col_size && m_col_size > 0, ("out of bound"));
	MATRIX<T> tmp(&m_inhr);
	if (cidx == 0) {
		tmp.grow_row(m_row_size);
		tmp.grow_col(*this, 0, m_col_size - 1);
		*this = tmp;
		return;
	}
	inner(tmp, 0, 0, m_row_size - 1, cidx - 1);
	tmp.grow_col(1);
	tmp.grow_col(*this, cidx, m_col_size - 1);
	*this = tmp;
}


/*
Insert the number 'cnum' of empty columns before 'cidx'.
*/
template <class T>
void MATRIX<T>::insert_cols_before(UINT cidx, UINT cnum)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(cidx < m_col_size && m_col_size > 0, ("out of bound"));
	MATRIX<T> tmp(&m_inhr);
	if (cnum == 0) {
		return;
	}
	if (cidx == 0) {
		tmp.grow_row(m_row_size);
		tmp.grow_col(cnum - 1);
		tmp.grow_col(*this, 0, m_col_size - 1);
		*this = tmp;
		return;
	}
	inner(tmp, 0, 0, m_row_size - 1, cidx - 1);
	tmp.grow_col(cnum);
	tmp.grow_col(*this, cidx, m_col_size - 1);
	*this = tmp;
}


/*
Inserting columns before 'cidx' which columns copy from matrix 'm'
from 'from' to 'to'.
*/
template <class T>
void MATRIX<T>::insert_cols_before(UINT cidx, IN MATRIX<T> const& m,
								   UINT mfrom, UINT mto)
{
	IS_TRUE(m_is_init && m.m_is_init, ("not yet initialize."));
	IS_TRUE(cidx < m_col_size && m_col_size > 0 &&
			mfrom < m.m_col_size && mto < m.m_col_size &&
			mfrom <= mto,	("out of bound"));
	MATRIX<T> tmp(&m_inhr);
	if (cidx == 0) {
		tmp.grow_row(m_row_size);
		tmp.grow_col(m, mfrom, mto);
		tmp.grow_col(*this, 0, m_col_size - 1);
		tmp.del_col(0);
		*this = tmp;
		return;
	}
	inner(tmp, 0, 0, m_row_size - 1, cidx - 1);
	tmp.grow_col(m, mfrom, mto);
	tmp.grow_col(*this, cidx, m_col_size - 1);
	*this = tmp;
}


/*
Get inner matrix at starting point
(sr,sl) in matrix, and end point (er,el).
*/
template <class T>
void MATRIX<T>::inner(MATRIX<T> & in,
					UINT sr, UINT sl, UINT er, UINT el) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(sr <= er && sl <= el && er < m_row_size && el < m_col_size,
			("illegal info"));
	UINT row = er - sr + 1;
	UINT col = el - sl + 1;
	in.reinit(row, col, &m_inhr);
	for (UINT i = sr; i <= er; i++) {
		for (UINT j = sl; j <= el; j++) {
			in.set(i-sr, j-sl, get(i,j));
		}
	}
}


/*
Get inner rows.
*/
template <class T>
void MATRIX<T>::inner_row(OUT MATRIX<T> & in, UINT from, UINT to) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(from <= to && to < m_row_size, ("illegal parameter"));
	inner(in, from, 0, to, m_col_size -1);
}


/*
Get inner columns.
*/
template <class T>
void MATRIX<T>::inner_col(OUT MATRIX<T> & in, UINT from, UINT to) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(from <= to && to < m_col_size, ("illegal parameter"));
	inner(in, 0, from, m_row_size - 1, to);
}


/*
Pivot Triangular Decomposition.
Compute P,L,U in terms of the formula: PA = LU,
where P:Permute MatrixL, L:Lower triangular matrix, U: upper triangular matrix.

NOTICE:
	1.The precision of 'T' may has serious effect.
*/
template <class T>
bool MATRIX<T>::plu(OUT MATRIX<T> & p, OUT MATRIX<T> & l, OUT MATRIX<T> & u)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 1 && m_col_size > 1, ("strange matrix"));
	bool succ = false;
	MATRIX<T> permd(&m_inhr); //Permuting this matrix by 'p'.
	UINT i;
	p.reinit(m_row_size, m_row_size, &m_inhr); //Permute Matrix
	p.eye(1);
	u = *this;

	for (UINT row = 0, col = 0;
			row < u.m_row_size && col < u.m_col_size;
			row++, 	col++) {
		//Finding the perfect pivot entry or permuting of non-zero pivot entry.
		INT swap_row = -1;
		T swap_entry;

		//Find column with non-zero entry. 'w' meas 'which'
		for (UINT w = col; w < u.m_col_size; w++) {
			for (UINT k = row; k < u.m_row_size; k++) {
				T tmp = u.get(k, w);
				if (_equal(tmp, T(0))) {
					continue;
				}
				if (swap_row == -1) {
					swap_row = k; //The first non-zero entry is the candidate
					swap_entry = tmp;
					if (swap_entry == 1) {
						break;
					}
				} else if (abs(swap_entry) < abs(tmp)) {
					//or always find the maximum entry as pivot entry.
					swap_row = k;
					swap_entry = tmp;
				}
			} //end for
			if (swap_row == -1) {
				continue; //try next column, trapezoid matrix
			}
			if (swap_row != (INT)row) {
				u.interch_row(swap_row, row);
				p.interch_row(swap_row, row); //Record permutate matrix
			}
			col = w; //Record current non-zero column
			break;
		} //end for
		if (swap_row == -1) {
			goto FIN; //All elements of row 'row' be zero.
		}

		//Reduce other column entries below pivot entry to zero.
		T topent = u.get(row, col);
		IS_TRUE(!_equal(topent, T(0)), ("zero entry"));
		for (i = row + 1; i < u.m_row_size; i++) {

			//For reducing the computation cost.
			if (!_equal(u.get(i, col), T(0))) {
				T tmp = -1 * u.get(i, col) / topent;
				u.mul_and_add_row(row, tmp, i);
			}
		} //end for
	} //end for (INT row = 0...

	permd = *this; //Permuting this matrix by 'p'.
	permd = p * permd;
	l.reinit(m_row_size, m_row_size, &m_inhr);
	l.eye(1);
	succ = permd.lu(l,u);
	if (succ) {
		/*
		In 'double' precison, the equation may be failed , the reason was
		caused by the precision. But the result is correct.
		//IS_TRUE(permd == l * u, ("unequal for PA=L*U after decomposition"));
		*/
	}
FIN:
	return succ;
}


/*
LU Decomposition.
Compute L,U in terms of the formula: A = LU,
where L:Lower triangular matrix, U: upper triangular matrix.

NOTICE:
	1.The precision of 'T' may has serious effect.
*/
template <class T>
bool MATRIX<T>::lu(OUT MATRIX<T> & l, OUT MATRIX<T> & u)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size > 1 && m_col_size > 1, ("strange matrix"));
	bool succ = false;
	UINT i;
	u = *this;
	l.reinit(m_row_size, m_row_size, &m_inhr);
	l.eye(1);

	for (UINT row = 0, col = 0;
			row < u.m_row_size && col < u.m_col_size;
			row++, 	col++) {
		//Finding the perfect pivot entry or permuting
		//of non-zero pivot entry.
		INT swap_row = -1;
		T swap_entry;

		//Find  column with non-zero entry. 'w' meas 'which'
		for (UINT w = col; w < u.m_col_size; w++) {
			for (UINT k = row; k < u.m_row_size; k++) {
				T tmp = u.get(k, w);
				if (_equal(tmp, T(0))) {
					continue;
				}
				if (swap_row == -1) {
					swap_row = k; //The first non-zero entry
								  //is the candidate.
					swap_entry = tmp;
					break;
				}
			}//end for
			if (swap_row == -1) {
				continue; //try next column, trapezoid matrix
			}
			if (swap_row != (INT)row) {
				/*
				Can not reduce to up triangular form.
				Theroem:
					There is no LU decomposition can be find if A
					has a singular sub matrix in upper-left corner.
				*/
				goto FIN;
			}
			col = w; //Record current non-zero column
			break;
		}//end for
		if (swap_row == -1) {
			goto FIN; //All elements of row 'row' be zero.
		}

		for (i = row; i < l.m_row_size; i++) {
			l.set(i, row, u.get(i, col));
		}
		//Reduce other column entries below pivot entry to zero.
		T topent = u.get(row, col);
		IS_TRUE(!_equal(topent, T(0)), ("zero entry"));
		for (i = row + 1; i < u.m_row_size; i++) {
			if (!_equal(u.get(i, col), T(0))) {
				T tmp = -1 * u.get(i, col) / topent;
				u.mul_and_add_row(row, tmp, i);
			}
		}//end for
	}//end for (INT row = 0...

	for (i = 0; i < l.m_row_size; i++) {
		/*
		Reduce pivot entry to unitary, dividing  sub-entry
		by pivot entry.
		*/
		T tmp = 1 / l.get(i, i);

		l.mul_of_col(i, tmp);
	}
	succ = true;
FIN:
	return succ;
}


/*
QR Decomposition.
Function produces an upper triangular matrix R of the same
dimension as row of A and a unitary matrix Q(orthonomal basis)
so that A = Q*R.

Space of row vector of A that can be descripte by number of basis,
also could be represented by number of orthogonal basis with
relatived coefficient.
And R consists of these coefficients.

'calc_basis' If true is means to first calculate basis of 'this',
and then compute the QR.

NOTICE:
	1.'this' uses row convention, so the formual need to transform.
		Orignal theroem: Given A uses col convention., and A=QR, so
		trans(Q)A = trans(Q)(QR) = R, here R is upper-triangular
		matrix. Now is, given A use row convention, there are two
		methods to get the Q,R, the first is:
			replace A in original formual with trans(A).
			so
			trans(trans(Q)*A) = trans(A)*Q = trans(Q*R)*Q =
			trans(R)*trans(Q)*Q = trans(R)

		and the second is:
			Because A use row convention,  Q is also use row convention.
			To get upper triangular R,
			we need projecting each axis of vectors in A, and the project
			coefficent of each axis is the row of Q, so we get the
			formaluar:
				R = Q * trans(A), Q is orthogonal, row convention,
				and each row indicate project coefficent.
			A uses column convention,  each column means vector of space A.
			Then the output 'q','r' is colQ, and colR, and in actually,
			colQ*colR uses col convention.

	2.'this' matrix should be row linearly independent as well,
		otherwise 'calc_basis' must set to 'true', then basis of A be
		computed, and the out put Q*R equals basis(A).

	3.The precision of 'T' may has serious effect.
*/
template <class T>
void MATRIX<T>::qr(OUT MATRIX<T> & q, OUT MATRIX<T> & r, bool calc_basis)
{
	IS_TRUE(m_is_init, ("not yet initialize."));

	//Linear dependent for some of row vectors.
	if (calc_basis && rank() < m_row_size - 1) {
		MATRIX<T> bs;
		basis(bs);
		bs.qr(q, r);
		return;
	}
	orth(q);  //Computing orthogonal basis.
	q.nml(); //Normalizing orthogonal basis.
#ifdef QR_M2 //Method2, see NOTICE.
	MATRIX<T> A = *this;
	A.trans();
	r =  q * A;

	//q is orthogonal basis set, and q should use col convention.
	//and trans(A) equals Q*R.
	q.trans();
#else //Method 1, see NOTICE.

	//q is orthogonal basis set, and q should use col convention.
	//and trans(A) equals Q*R.
	q.trans();
	r =  *this * q;
	r.trans();
#endif
}


/*
Normalize each row vector.

NOTICE:
	'this' uses row convention.
*/
template <class T>
void MATRIX<T>::nml()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	for (UINT i = 0; i < m_row_size; i++) {
		T modv = modrow(i);
		if (_equal(modv, T(0))) {
			continue;
		}
		mul_of_row(i, 1/modv);
	}
}


/*
Permute QR Decomposition.
Function produces a permutation matrix p, an upper triangular
matrix R of the same dimension as A and a unitary matrix Q so that AP = Q*R.
The column permutation p is chosen so that abs(diag(R)) is decreasing.

NOTICE:
	1.The precision of 'T' may has serious effect.
*/
template <class T>
bool MATRIX<T>::pqr(MATRIX<T> & p, MATRIX<T> & q, MATRIX<T> & r)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	bool succ = false;
	IS_TRUE(0, ("TODO"));
FIN:
	return succ;
}


/*
Compute Dot Product between referred 'row' and 'v'.
*/
template <class T>
T MATRIX<T>::dotrow(UINT row, IN MATRIX<T> const& v) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(v.is_vec() && v.size() == m_col_size, ("not a vector."));
	return dot(row, 0, row, m_col_size - 1, v);
}


/*
Compute Dot Product between referred 'col' and 'v'.
*/
template <class T>
T MATRIX<T>::dotcol(UINT col, IN MATRIX<T> const& v) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(v.is_vec() && v.size() == m_row_size, ("not a vector."));
	return dot(0, col, m_row_size - 1, col, v);
}


/*
Compute Dot Product.
v: v must be a row/col vector.
*/
template <class T>
T MATRIX<T>::dot(IN MATRIX<T> const& v) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(size() == v.size(), ("not a vector."));
	IS_TRUE(v.m_row_size == 1 || v.m_col_size == 1,
			("not a vector, only one row or one col"));
	IS_TRUE(m_row_size ==1 || m_col_size == 1,
			("not a vector, only one row or one col"));
	T dotv = 0;
	if (m_row_size == 1 && v.m_row_size == 1) {
		//'this' is row vector, 'v' is row vector
		for (UINT i = 0; i < m_col_size; i++) {
			T v1 = get(0,i);
			T v2 = v.get(0,i);
			dotv = dotv + (get(0, i) * v.get(0,i));
		}
	} else if (m_row_size == 1 && v.m_col_size == 1) {
		//'this' is row vector, 'v' is col vector
		for (UINT i = 0; i < m_col_size; i++) {
			dotv = dotv + (get(0, i) * v.get(i,0));
		}
	} else if (m_col_size == 1 && v.m_col_size == 1) {
		//'this' is col vector, 'v' is col vector
		for (UINT i = 0; i < m_row_size; i++) {
			dotv = dotv + (get(i,0) * v.get(i,0));
		}
	} else if (m_col_size == 1 && v.m_row_size == 1) {
		//'this' is col vector, 'v' is row vector
		for (UINT i = 0; i < m_row_size; i++) {
			dotv = dotv + (get(i,0) * v.get(0,i));
		}
	} else {
		IS_TRUE(0,("illegal operation"));
	}
	return dotv;
}


/*
'srow': starting row
'scol': starting column
'erow': ending row
'ecol': ending column
'v': value matrix, must be a row/col vector.

NOTICE:
	'srow, scol, erow, ecol'  must express one vector.
*/
template <class T>
T MATRIX<T>::dot(UINT srow, UINT scol,
				 UINT erow, UINT ecol, IN MATRIX<T> const& v) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(srow <= erow && scol <= ecol &&
					(erow - srow + 1) * (ecol - scol + 1) == v.size(),
					("not a vector."));
	IS_TRUE(srow < m_row_size  && scol < m_col_size &&
			erow < m_row_size  && ecol < m_col_size, ("out of boundary."));
	IS_TRUE(v.m_row_size ==1 || v.m_col_size == 1,
			("not a vector, only one row or one col"));
	IS_TRUE(srow == erow || scol == ecol,
			("not a vector, only one row or one col"));

	T dotv = 0;
	if (srow == erow && v.m_row_size == 1) {
		//'this' is row vector, 'v' is row vector
		for (UINT i = scol; i <= ecol; i++) {
			dotv = dotv + (get(srow, i) * v.get(0,i));
		}
	} else if (scol == ecol && v.m_row_size == 1) {
		//'this' is col vector, 'v' is row vector
		for (UINT i = srow; i <= erow; i++) {
			dotv = dotv + (get(i,scol) * v.get(0,i));
		}
	} else if (scol == ecol && v.m_col_size == 1) {
		//'this' is col vector, 'v' is col vector
		for (UINT i = srow; i <= erow; i++) {
			dotv = dotv + (get(i,scol) * v.get(i,0));
		}
	} else if (srow == erow && v.m_col_size == 1) {
		//'this' is row vector, 'v' is col vector
		for (UINT i = scol; i <= ecol; i++) {
			dotv = dotv + (get(srow,i) * v.get(i,0));
		}
	} else {
		IS_TRUE(0,("illegal operation"));
	}
	return dotv;
}


/*
Compute Dot Product between.
srow: starting row, scol:starting column, erow:end row, ecol:end column.

'srow, scol, erow, ecol'  must express one vector.
'v': v must be a row/col vector.
*/
template <class T>
T MATRIX<T>::dot(UINT srow, UINT scol, UINT erow, UINT ecol,
				IN MATRIX<T> const& v, UINT vsrow, UINT vscol,
				UINT verow, UINT vecol) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(srow <= erow &&
			scol <= ecol &&
			vsrow <= verow &&
			vscol <= vecol &&
			(erow - srow + 1) * (ecol - scol + 1) ==
			  (verow - vsrow + 1) * (vecol - vscol + 1),
			("not a vector."));
	IS_TRUE(srow < m_row_size  && scol < m_col_size &&
					vsrow < v.m_row_size  && vscol < v.m_col_size &&
					erow < m_row_size  && ecol < m_col_size &&
					verow < v.m_row_size  && vecol < v.m_col_size,
			("out of boundary."));
	IS_TRUE(srow == erow || scol == ecol,
			("not a vector, only one row or one col"));
	IS_TRUE(vsrow == verow || vscol == vecol,
		("not a vector, only one row or one col"));

	T dotv = 0;

	//'this' is row vector, 'v' is row vector
	if (srow == erow && vsrow == verow) {
		for (UINT i = scol; i <= ecol; i++) {
			dotv = dotv + (get(srow, i) * v.get(vsrow,i));
		}
	} else if (srow == erow && vscol == vecol) {
		//'this' is row vector, 'v' is col vector
		for (UINT i = scol; i <= ecol; i++) {
			dotv = dotv + (get(srow,i) * v.get(i,vscol));
		}
	} else if (scol == ecol && vsrow == verow) {
		//'this' is col vector, 'v' is row vector
		for (UINT i = srow; i <= erow; i++) {
			dotv = dotv + (get(i,scol) * v.get(vsrow,i));
		}
	} else if (scol == ecol && vscol == vecol) {
		//'this' is col vector, 'v' is col vector
		for (UINT i = srow; i <= erow; i++) {
			dotv = dotv + (get(i,scol) * v.get(i,vscol));
		}
	} else {
		IS_TRUE(0,("illegal operation"));
	}
	return dotv;
}


/*
Vector Product(cross product).
Given two vector [a1, b1, c1], [a2, b2, c2], the vector product is:
[b1*c2-c1*b2, c1*a2-a1*c2, a1*b2-b1*a2].

'v': v must be row/col vector with three dimensions.
'u': cross product
*/
template <class T>
void MATRIX<T>::cross(IN MATRIX<T> & v, OUT MATRIX<T> & u)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(&u != this,
			("output matrix is same with current one"));
	IS_TRUE(m_row_size == 1 || m_col_size == 1,
			("not a vector, only one row or one col"));
	if (&v == this) {
		u.reinit(1, MAX(v.get_col_size(), v.get_row_size()));
		u.zero();
		return;
	}

	if (m_row_size == 1) {
		//'this' is row vector
		if (v.m_row_size == 1) {
			//'v' is row vector
			v.trans();
			cross(v, u);
			v.trans();
		} else {
			//'v' is col vector
			IS_TRUE(m_col_size == 3 && v.get_row_size() == 3,
					("only valid for 3-dimension vector"));
			u.reinit(1, v.get_row_size());
			u.set(0, 0, get(0, 1)*v.get(2, 0) - get(0, 2)*v.get(1, 0));
			u.set(0, 1, get(0, 2)*v.get(0, 0) - get(0, 0)*v.get(2, 0));
			u.set(0, 2, get(0, 0)*v.get(1, 0) - get(0, 1)*v.get(0, 0));
		}
	} else {
		//'this' is col vector
		trans();
		cross(v, u);
		trans();
	}
}


/*
Orthogonalization and Normalization by Gram-Schmidt method.

'z': use row convention.

NOTICE:
	'this' is a row vector
 */
template <class T>
void MATRIX<T>::orthn(OUT MATRIX<T> & z)
{
	IS_TRUE(m_is_init, ("not yet initialize."));

	MATRIX<T> b1;
	z.reinit(m_row_size, m_col_size, &m_inhr);
	inner_row(b1, 0, 0); //first row vector.
	T dotv = b1.dot(b1);
	b1.mul_of_row(0, _sqrt(1/dotv)); //normalize first dim.
	z.set_row(0, b1);
	for (UINT row = 1; row < m_row_size; row++) {
		//Orthogonalizing: <Xn,Zi>*Zi
		MATRIX<T> allzi(1,z.m_col_size, &m_inhr);
		for (INT i = row-1; i >= 0; i--) { //Here index 'i' must be signed
			MATRIX<T> zi;
			z.inner_row(zi, i, i); //(k-1)th~0th basis
			T dv = dot(row, 0, row, m_col_size-1, zi);
			zi.mul(dv);
			allzi = allzi + zi;
		}
		MATRIX<T> xi;
		inner_row(xi, row, row); //kth vector.

		//Normalization: Zi = Zi/|Zi|
		MATRIX<T> no = xi - allzi;
		no.mul(1/_sqrt(no.dot(no)));
		z.set_row(row, no);
	}
}


/*
Orthogonalization by Gram-Schmidt method.

'z': Each row of vectors are orthogonalized mutually.
	It uses row convention.

NOTICE:
	'this' use row convention.
	Each rows of vectors should be independent mutually.
	Or else zero vector	was generated.
 */
template <class T>
void MATRIX<T>::orth(OUT MATRIX<T> & z)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	MATRIX<T> b1;
	z.reinit(m_row_size, m_col_size, &m_inhr);
	inner_row(b1, 0, 0); //first row vector
	z.set_row(0, b1);
	for (UINT row = 1; row < m_row_size; row++) {
		//Only orthogonalize: (<Xn,Zi>/<Zi,Zi>)*Zi
		MATRIX<T> allzi(1,z.m_col_size, &m_inhr);
		for (INT i = row - 1; i >= 0; i--) { //'i' must be signed
			MATRIX<T> zi;
			z.inner_row(zi, i, i); //(k-1)th~0th basis
			T dv_zz = zi.dot(zi);

			/*
			CASE: If two rows of elements are equal, such as,
					[a,b,c]
					[1,1,1]
					[1,1,1]
				'dv_zz' will be zero!
			*/
			IS_TRUE(dv_zz != 0,
					("dot() is zero, row '%d' migth be all zero", i));
			T dv_xz = dot(row, 0, row, m_col_size-1, zi) / dv_zz;
			zi.mul(dv_xz);
			allzi = allzi + zi;
		}
		MATRIX<T> xi;
		inner_row(xi, row, row); //kth vector.

		//Store the component vector as the kth orthogonal
		//axis of row convention space.
		z.set_row(row, xi - allzi);
	}
}


/*
Methods of  least squares to get approximate solution.

'x': result of system equations
	X = x1*v1 + x2*v2 + ...	xn*vn + c,  X is n*n if result has no
	constant vector,
	or X is n*(n+1), the column n+1 is constant vector

	The (b - A*x) is the least square error.
	If A is invertable, A*x equals 'b'.

'b': constant vector.

NOTICE:
	A is row convention, and each column indicate  the unknown.
*/
template <class T>
void MATRIX<T>::mls(OUT MATRIX<T> & x, IN MATRIX<T> const& b)
{
	//Method of least squares: trans(A)*A*x = trans(A)*b
	MATRIX<T> A = *this;
	A.trans();
	MATRIX<T> a = A * b; //trans(A)*b, a is vector
	A = A * *this; //trans(A)*A
	A.grow_col(a); //a is augment coefficient vector

	//For speed up, we do invoke null() instead of sse(). It's only a trick.
	A.null(x);
	x.del_row(x.m_row_size - 1); //Only number of (m_row_size - 1) unknowns.
}


/*
Solving system of linear equations.
Return true if the solution is unique, othwise return false, and x is
generating solution.

'x': result of system equations
	1.Unique solution: [v1, v2, ... vn],  and 'x' is column vector.
	2.Generating solutin:
		x1*[a1,...,an] + x2*[b1,...,bn] + ... + xn*[k1,...,kn] + [C1,...,Cn]
		x1,x2,...,xn are unconstrained variables.

	'x' is n*(n+1) matrix, last column is augmented vector.

'b': augmented vector.
	At present constant part only supports the constant value.

NOTICE:
	1. A is invertable, so is LU decomposable as well, the result is unique.
		To get approximatest solution, call mls().
	2. A is row convention, and each column indicate the unknown.
	3.If 'this' is singular, compute the general solution utilize null().
*/
template <class T>
bool MATRIX<T>::sse(OUT MATRIX<T> & x, IN MATRIX<T> const& b)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size == b.m_row_size, ("unmatch format"));

	//TODO: Constant part should permit the support of
	//constant symbol as well.
	IS_TRUE(b.is_colvec(), ("b must be col vector"));

	//Theorem: PA = LU, Ly = Pb, Ux = y
	if (det() != 0) {
		MATRIX<T> p,l,u;
		bool s = plu(p,l,u);
		IS_TRUE(s, ("illegal solution"));
		u.inv(u);
		l.inv(l);
		x = u * l * p * b;
		return true;
	}

	/*
	det() is zero, there are two case:
	1.no solution 2.infinitely many solutions
	*/

	//Computing general solution.
	MATRIX<T> tmp = *this;

	/*
	Here we use the function null() to reduce and format one
	generating solution.
	However, function null() regard the last column of 'tmp' as
	one variable when we invoke grow_col(b).
	Consequently, to get the correct solution from output of null(), the
	sign of the last column must be negative value of original.
	*/
	tmp.grow_col(b);
	tmp.null(x);

	//There are number of (m_row_size - 1) unknowns.
	x.del_row(x.m_row_size - 1);
	x.mul_of_col(x.get_col_size() - 1, -1);
	return false;
}


/*
Compute Modular of vector
*/
template <class T>
T MATRIX<T>::mod()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_vec(), ("not a vector, only one row or one col"));
	T modv = 0;

	if (is_rowvec()) { //'this' is row vector
		for (UINT i = 0; i < m_col_size; i++) {
			T v = get(0, i);
			modv = modv + (v * v);
		}
	} else if (is_colvec()) { //'this' is col vector
		for (UINT i = 0; i < m_row_size; i++) {
			T v = get(i,0);
			modv = modv + (v * v);
		}
	} else {
		IS_TRUE(0,("illegal operation"));
	}
	modv = _sqrt(modv);
	return modv;
}


/*
Compute Modular of one of row
*/
template <class T>
T MATRIX<T>::modrow(UINT row)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(row < m_row_size, ("out of boundary"));
	T modv = 0;
	for (UINT i = 0; i < m_col_size; i++) {
		T v = get(row, i);
		modv = modv + (v * v);
	}
	modv = _sqrt(modv);
	return modv;
}


/*
Compute Modular of one of col
*/
template <class T>
T MATRIX<T>::modcol(UINT col)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(col < m_col_size, ("out of boundary"));
	T modv = 0;
	for (UINT i = 0; i < m_row_size; i++) {
		T v = get(i, col);
		modv = modv + (v * v);
	}
	modv = _sqrt(modv);
	return modv;
}


/*
Computation of eigenvalues.

'eigv': is a vector containing the eigenvalues of a square matrix.

NOTICE:
	'this' matrix must be row-vector matrix
*/
template <class T>
void MATRIX<T>::eig(OUT MATRIX<T> & eigv)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(), ("A must be square."));
	MATRIX<T> v = *this;
	MATRIX<T> q,r;
	UINT count = 0;
	while (!v.is_low_triangular() && count < MAX_QR_ITER_NUM) {
#ifdef QR_ITER_WITH_SHIFT
		MATRIX<T> i(m_row_size, m_col_size, &m_inhr);
		i.eye(v.get(v.m_row_size -1 , v.m_col_size - 1));
		v = v - i;
		v.qr(q,r);
		v = r*q;
		v.trans(); //v must use row convention.
		v = v + i;
#else
		v.qr(q,r);
		v = r*q;
		v.trans(); //v must use row convention.
#endif
		count++;
	}
	IS_TRUE(v.is_quad() && v.m_row_size == m_row_size, ("A must be square."));

	UINT i;
	//Sort diagonal eigvalue in decreasing order
	for (i = 0; i < v.m_col_size - 1; i++) {
		T vi = v.get(i, i);
		for (UINT j = i + 1; j < v.m_col_size; j++) {
			T vj = v.get(j, j);
			if (vi < vj) { //swap elements
				T tmp = vi;
				v.set(i, i, vj);
				v.set(j, j, tmp);
			}
		}
	}

	eigv.reinit(1, m_col_size, &m_inhr);
	for (i = 0; i < m_row_size; i++) {
		eigv.set(0, i, v.get(i, i));
	}
}


/*
Computation of eigenvalues and eigenvectors.

'eigv': is a matrix containing the eigenvalues of a square matrix.
	It is diagonal matrix

'eigx': is a full matrix whose columns are the corresponding
	eigenvectors so that A*eigX = eigX*eigV.
	It uses col convention.

NOTICE:
	'this' matrix must be row-vector matrix
*/
template <class T>
void MATRIX<T>::eig(OUT MATRIX<T> & eigv, OUT MATRIX<T> & eigx)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(), ("A must be square."));
	MATRIX<T> v;
	eig(v);
	v.adjust();
	eigv.reinit(v.size(), v.size(), &m_inhr);
	eigv.setdiag(v);
	eigx.reinit(0, 0, &m_inhr);
	LIST<T> vlst;
	for (UINT i = 0; i <  eigv.m_row_size; i++) {
		T lam = eigv.get(i,i);
		if (vlst.find(lam)) {
			continue;
		}
		vlst.append_tail(lam);

		//Computing nulA of ('this'-lam*I)
		MATRIX<T> x(m_row_size, m_col_size, &m_inhr), nul;
		x.eye(lam); //lam*I
		x = *this - x;
		x.null(nul); //null space descripted in col convention.
		//Extract the non-zero col basis from null space of (A-lam*I)
		for (UINT j = 0; j < nul.m_col_size; j++) {
			bool zero_col = true;
			for (UINT w = 0; w < nul.m_row_size; w++) {
				if (!_equal(nul.get(w, j), T(0))) {
					zero_col = false;
					break;
				}
			}
			if (!zero_col) {
				MATRIX<T> tmpcol;
				nul.inner_col(tmpcol, j, j);
				eigx.grow_col(tmpcol);
			}
		}//end for
	}//end for
}


/*
Computation of projection of vector 'v' in row space of 'this'.

'p': projection of v
'v': vector to project .

NOITCE: 1. each row of 'this' must be orthogonal base of one space.
		2. 'this' uses row convention.
*/
template <class T>
void MATRIX<T>::proj(OUT MATRIX<T> & p, IN MATRIX<T> const& v)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_orth(), ("need orthogonal"));
	IS_TRUE(v.is_vec() && v.size() == m_col_size, ("must be vector"));
	p.reinit(1, m_col_size, &m_inhr);
	for (UINT i = 0; i < m_row_size; i++) {
		MATRIX<T> row;
		inner_row(row, i, i);
		T dv = v.dot(row);
		if (_equal(dv, T(0))) {
			continue;
		}
		dv = dv / row.dot(row);
		row.mul(dv);
		p = p + row;
	}
}


/*
Strange Value decomposition
A = u*s*eigx, 'this' is m*n matrix.

'u': is m*m orthonormal matrix, col vector form arrangement.
's': m*n stanger value matrix, diagonal matrix.
'eigx': n*n  orthonormal matrix,  row vector form arrangement.
*/
template <class T>
bool MATRIX<T>::svd(OUT MATRIX<T> & u, OUT MATRIX<T> & s, OUT MATRIX<T> & eigx)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	if (m_row_size < m_col_size) {
		/*
		Assuming that A is 2*3, then B=trans(A)*A is  3*3 matrix.
		When we computing SVD of trans(A) instead of A, and B is A*trans(A),
		the 2*2 matrix.
		It is simpler to compute the EIGX,EIGV than those of A.
		Because A=USV, so trans(A) = trans(V)*trans(S)*trans(U).
		*/
		MATRIX<T> transA = *this;
		transA.trans();
		bool res = transA.svd(u, s, eigx);
		eigx.trans();
		u.trans();
		s.trans();
		MATRIX<T> tmp = eigx; eigx = u;	u = tmp;
		return res;
	}
	MATRIX<T> a, eigv, tmpeigx;
	a = *this;
	a.trans();
	a = a * *this;
	a.adjust();
	a.eig(eigv, tmpeigx); //tmpeigx uses col convention.

	//Eigvalues already be in decreasing order.
	tmpeigx.trans();
	tmpeigx.orthn(eigx); //eigx use row convention.
	eigx.adjust();

	if (eigx.m_row_size != this->m_col_size) {
		return false;
	}

	if (!eigx.is_quad()){ //eigx must be n*n
		return false;
	}

	//element of 's' must be initialized with 0
	s.reinit(m_row_size, m_col_size, &m_inhr);

	//Computing stranger value s.
	//s = sqrt (eigen value);
	UINT scount = 0, i; //count of stranger value
	for (i = 0; i < MIN(s.m_row_size, eigv.m_row_size); i++) {
		T sv = eigv.get(i, i);
		if (_equal(sv, T(0))) {
			break;
		}
		scount++;
		s.set(i, i, _sqrt(sv));
	}

	//Computing Av1 and normalized it.
	MATRIX<T> v, subU; //subU uses col convention.
	for (i = 0; i < scount; i++) {
		eigx.inner_row(v, i, i); //Get the 1th dimension axis.
		v.trans(); //v1 must be col vector
		MATRIX<T> tmp = *this * v;  //tmpu1 is col vector

		//tmpu1 is 1th orthonormal basis of A, and also the first col vector
		//of U. Delta1 is modular of Av1.
		tmp.mul(1/s.get(i, i));

		subU.grow_row(tmp);
	}

	/*
	To construct U, we need the quad-matrix to present R^n space,
	and the first rth axis composed 'subU'.
	So we compute the NulA of u1, namely, the result
	of 'RowA(subU) * x = 0'.
	*/
	if (scount < m_row_size) {
		MATRIX<T> nulofu1, tmpu;
		subU.null(nulofu1); //nulofu1 uses col convention.

		//Compose U, with row convention arrangement.
		for (i = 0; i < nulofu1.m_col_size; i++) {
			if (!nulofu1.is_colequ(i, 0)) { //
				MATRIX tmp;
				nulofu1.inner_col(tmp, i, i);
				subU.grow_row(tmp);
			}
		}
		IS_TRUE(subU.is_quad(), ("too many basis"));
	}

	//And then subU has orthogonalized u2, u3, ..., we need farther
	//orthogonalize u2, u3,... each other.
	subU.orthn(u); //subU use row convention, u use row convention.
	u.trans();
	return true;
}


/*
Diagonalize matrix, form as A = P*D*inv(P), P is eigen space of eahc
eigen value.
D is diagonal matrix that diagonal entry is eigen value.

Return true if it is diagonalizable, otherwise return false.

'p': eigen space matrix, each column correspond to related eigen value.
	It uses col convention..
'd': eigen value matrix. It was diagonally.

NOTICE:
	'this' use row convention.
*/
template <class T>
bool MATRIX<T>::diag(OUT MATRIX<T> & p, OUT MATRIX<T> & d)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(is_quad(), ("only quad matirx"));
	eig(d, p); //d is diagonal, p uses col convention.
	if (!p.is_quad()) {
		//There were not enough eigen vectors to construct the basis of R^n.
		return false;
	}
	return true;
}


/*
Calculate matrix/vector norm.

'p': can be 1, 2, infinite.

NOTICE:
	1. 'this' uses row convention.
*/
template <class T>
T MATRIX<T>::norm(INT p)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	T n;
	if (p == NORM_F) {
		n = 0;
		for (UINT i = 0; i < m_row_size; i++) {
			for (UINT j = 0; j < m_col_size; j++) {
				n = n + (get(i, j) * get(i, j));
			}
		}
		return _sqrt(n);
	} else if (p == NORM_1) {
		if (is_vec()) { //vector norm
			n = 0;
			for (UINT i = 0; i < m_row_size; i++) {
				for (UINT j = 0; j < m_col_size; j++) {
					n = n + abs(get(i, j));
				}
			}
		} else { //matirx norm, row norm
			n = 0;
			for (UINT i = 0; i < m_row_size; i++) {
				T tmp = 0;
				for (UINT j = 0; j < m_col_size; j++) {
					tmp = tmp + abs(get(i, j));
				}
				n = MAX(n, tmp);
			}
		}
		return n;
	} else if (p == NORM_2) {
		if (is_vec()) { //vector norm
			n = 0;
			for (UINT i = 0; i < m_row_size; i++) {
				for (UINT j = 0; j < m_col_size; j++) {
					n = n + (get(i, j) * get(i, j));
				}
			}
			return _sqrt(n);
		} else { //matrix norm
			MATRIX<T> tmp = *this;
			tmp.trans();
			return _sqrt((tmp * *this).sprad());
		}
	} else if (p == NORM_INF) {
		if (is_vec()) { //vector norm
			n = 0;
			for (UINT i = 0; i < m_row_size; i++) {
				for (UINT j = 0; j < m_col_size; j++) {
					n = MAX(n, abs(get(i, j)));
				}
			}
		} else { //matrix norm, column norm
			n = 0;
			for (UINT j = 0; j < m_col_size; j++) {
				T tmp = 0;
				for (UINT i = 0; i < m_row_size; i++) {
					tmp = tmp + abs(get(i, j));
				}
				n = MAX(n, tmp);
			}
		}
		return n;
	}
	IS_TRUE(0, ("unknown norm type"));
	return 0;
}


/*
Condition number for Frobenius Norm.

k(A) = norm(A) * norm(inv(A))

Return true if 'this' is nonsingular matrix.
*/
template <class T>
bool MATRIX<T>::cond(T & c, INT p)
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	MATRIX<T> m;
	if (!inv(m)) {
		return false;
	}
	c = norm(p)*m.norm(p);
	return true;
}


/*
Spectral radius

Specrad = max|lamda(i)|, i=1~N-1

NOTICE:
	'this' matrix must be row-vector matrix
*/
template <class T>
T MATRIX<T>::sprad()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	MATRIX<T> v;
	eig(v);
	T maxeigv = 0;
	for (UINT j = 0; j < m_col_size; j++) {
		if (j == 0) {
			maxeigv = abs(v.get(0, j));
		} else {
			maxeigv = MAX(maxeigv, abs(v.get(0, j)));
		}
	}
	return maxeigv;
}


/*
Complement remainding rows for partial matrix to generate nonsingular matrix.
To obtain an invertible square matrix, we extend 'this' to a basis by simply
adding linearly independent unit vectors at the bottom of 'this'.each of which
spawns one missing dimension.
e.g:
	Given matrix is [1,1], the output matrix is [1,1][0,1]

NOTICE:
	The partial rows of matrix must be independent.
*/
template <class T>
void MATRIX<T>::padding()
{
	IS_TRUE(m_is_init, ("not yet initialize."));
	IS_TRUE(m_row_size != m_col_size && m_row_size != 0 && m_col_size != 0,
				("already nonsin"));

	MATRIX<T> tmp = *this;
	bool tran = false;
	if (tmp.m_col_size < tmp.m_row_size) {
		tmp.trans();
		tran = true;
	}
	UINT rows = tmp.m_row_size;
	tmp.eche();
	IS_TRUE(rows == tmp.m_row_size, ("rows vector are non independent"));

	for (UINT i = 0; i < tmp.m_row_size; i++) {
		for (UINT j = 0; j < tmp.m_col_size; j++) {
			if (tmp.get(i,j) != 0) {
				/*
				Set followed element to zero.
				e.g: row is [0,2,4,-1], then set last two be zero [0,2,0,0]
				*/
				for (UINT k = j+1; k < tmp.m_col_size; k++) {
					tmp.set(i,k,0);
				}
				break;
			}
		}
	}

	MATRIX<T> newrows(&m_inhr);
	for (UINT j = 0; j < tmp.m_col_size; j++) {
		bool allzero = true;
		for (UINT i = 0; i < tmp.m_row_size; i++) {
			if (tmp.get(i,j) != 0) {
				allzero = false;
				break;
			}
		}
		if (allzero) {
			if (newrows.m_row_size == 0) {
				newrows.grow_col(tmp.m_col_size);
			} else {
				newrows.grow_row(1);
			}
			newrows.set_row(newrows.m_row_size - 1, T(0));
			newrows.set(newrows.m_row_size - 1, j, 1);
		}
	}

	IS_TRUE(newrows.m_row_size == abs(m_row_size - m_col_size),
			 ("The partial rows of matrix must be independent."));

	if (tran) {
		newrows.trans();
		grow_col(newrows, 0, newrows.m_col_size - 1);
	} else {
		grow_row(newrows, 0, newrows.m_row_size - 1);
	}
}


template <class T>
void MATRIX<T>::dumpf(FILE * h) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
#ifdef NO_BASIC_MAT_DUMP
	if (HOOK_DUMPFH != NULL) {
		(*HOOK_DUMPFH)((void*)this, h);
		return;
	}
	IS_TRUE(0, ("dump not yet initialize."));
#else

	//Customization
	if (HOOK_DUMPFH != NULL) {
		(*HOOK_DUMPFH)((void*)this, h);
		return;
	}

	//printf("WARNING: dumpf should be overloaded!!!");

	//Default version regards T as 'int'.
	static INT g_count = 0;
	IS_TRUE(h, ("In dumpf(), file handle is NULL!!!"));
	fprintf(h, "\nMATRIX dump id:%d\n", g_count++);

	for (UINT i = 0; i < m_row_size; i++) {
		fprintf(h, "\t");
		for (UINT j = 0; j < m_col_size; j++) {
			CHAR *blank="           ";
			fprintf(h, "%10d%s", get(i,j), blank);
		}
		fprintf(h, "\n");
	}
	fprintf(h, "\n");
#endif
}


template <class T>
void MATRIX<T>::dumpf(CHAR * name, bool is_del) const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
#ifdef NO_BASIC_MAT_DUMP
	if (HOOK_DUMPF != NULL) {
		(*HOOK_DUMPF)((void*)this, name, is_del);
		return;
	}
	IS_TRUE(0, ("dump not yet initialize."));
#else

	//Customization
	if (HOOK_DUMPF != NULL) {
		(*HOOK_DUMPF)((void*)this, name, is_del);
		return;
	}

	//printf("WARNING: dumpf should be overloaded!!!");

	//Default version regards T as 'int'.
	if (!name) {
		name = "matrix.tmp";
	}
	if (is_del) {
		unlink(name);
	}
	FILE * h = fopen(name, "a+");
	IS_TRUE(h, ("%s create failed!!!", name));
	dumpf(h);
	fclose(h);
#endif
}


/*
Print as real number even though T is integer.
*/
template <class T>
void MATRIX<T>::dumps() const
{
	IS_TRUE(m_is_init, ("not yet initialize."));
#ifdef NO_BASIC_MAT_DUMP
	if (HOOK_DUMPS != NULL) {
		(*HOOK_DUMPS)((void*)this);
		return;
	}
	IS_TRUE(0, ("dump not yet initialize."));
#else

	//Customization
	if (HOOK_DUMPS != NULL) {
		(*HOOK_DUMPS)((void*)this);
		return;
	}

	//printf("WARNING: dumps should be overloaded!!!");

	//Default version regards T as 'int'.
	printf("\n");
	for (UINT i = 0; i < m_row_size; i++) {
		printf("\t");
		for (UINT j = 0; j < m_col_size; j++) {
			CHAR * blank="           ";
			T v = get(i,j);
			printf("%5d%s", v, blank);
		}
		printf("\n");
	}
	printf("\n");
#endif
}
//END MATRIX
#endif

