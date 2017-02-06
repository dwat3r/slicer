#ifndef RELATION_OPS_H
#define RELATION_OPS_H

#include <set>

/* Binary operations on
 * the relations.
*/

// Cartesian product for
// N x N
template<typename LHS,typename RHS>
inline std::set<std::pair <LHS,RHS> >
cart_prod(const std::set<LHS> lhs,
          const std::set<RHS> rhs){
  std::set<std::pair <LHS,RHS> > ret;
  for (auto l : lhs){
      for (auto r : rhs){
          ret.insert({l,r});
        }
    }
  return ret;
}

// 1 x N
template<typename LHS,typename RHS>
inline std::set<std::pair<LHS,RHS> >
cart_prod(const LHS lhs,
          const std::set<RHS> rhs){
  std::set<std::pair <LHS,RHS> > ret;
  for (auto r : rhs){
      ret.insert({lhs,r});
    }
  return ret;
}

// N x 1
template<typename LHS,typename RHS>
inline std::set<std::pair<LHS,RHS> >
cart_prod(const std::set<LHS> lhs,
          const RHS rhs){
  std::set<std::pair <LHS,RHS> > ret;
  for (auto l : lhs){
      ret.insert({l,rhs});
    }
  return ret;
}

// relation composition
template<typename LHS,typename MID,typename RHS>
inline std::set<std::pair <LHS,RHS> >
rel_comp(const std::set<std::pair<LHS,MID> > lhs,
         const std::set<std::pair<MID,RHS> > rhs){
  std::set<std::pair <LHS,RHS> > ret;
  for (auto l : lhs){
      for (auto r : rhs){
          if(l.second == r.first)
            ret.insert({l.first,r.second});
        }
    }
  return ret;
}

// union
// took from:
// http://stackoverflow.com/questions/22383474/c-lazy-way-to-find-union-of-stdsets
template <typename T>
inline std::set<T>& operator+=(std::set<T>& lhs, const std::set<T>& rhs){
    lhs.insert(begin(rhs), end(rhs));
    return lhs;
}

template <typename T>
inline std::set<T> operator+(std::set<T> lhs, const std::set<T>& rhs){
    lhs += rhs;
    return lhs;
}

/* reflexive and transitive closure
 * The algorithm is:
 * First insert the reflexive ones
 * and count distinct elements.
 *
 * Then use matrix multiplication,
 * which is in this case is raising the
 * adjacency matrix
 * to the number of distinct elements
 * in the relation,
 * which can be done by relational composition.
 */
template <typename T>
inline std::set< std::pair<T,T> >
trans_clos(std::set<std::pair<T,T> > s){
  auto ret(s);

  // first insert reflexive ones (and count unique elements)
  std::set<T> uniq_elems;
  for (auto v : s){
      ret.insert({v.first,v.first});
      ret.insert({v.second,v.second});
      uniq_elems.insert(v.first);
      uniq_elems.insert(v.second);
    }
  auto base(ret);
  auto curr(ret);
  for(int i=0;i<uniq_elems.size();++i){
      curr = rel_comp(curr,base);
      ret += curr;
    }
  return ret;
}

#endif // RELATION_OPS_H
