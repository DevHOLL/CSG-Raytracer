#ifndef _UNION_CSG_NODE_HPP_
#define _UNION_CSG_NODE_HPP_

#include "AbstractCsgNode.hpp"

namespace RT
{
  class UnionCsgNode : public RT::AbstractCsgNode
  {
  public:
    UnionCsgNode();
    virtual ~UnionCsgNode();

    virtual std::list<RT::Intersection>	render(RT::Ray const &, unsigned int) const override;	// Render sub-tree
  };
};

#endif
