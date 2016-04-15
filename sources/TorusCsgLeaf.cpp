#include <algorithm>
#include <sstream>

#include "Math.hpp"
#include "TorusCsgLeaf.hpp"

RT::TorusCsgLeaf::TorusCsgLeaf(double r, double h)
  : _r(r), _h(h)
{}

RT::TorusCsgLeaf::~TorusCsgLeaf()
{}

std::vector<double>	RT::TorusCsgLeaf::intersection(RT::Ray const & ray) const
{
  // Not so clever equations
  std::vector<double>	result = Math::Utils::solve(
    std::pow(ray.d().x() * ray.d().x() + ray.d().y() * ray.d().y() + ray.d().z() * ray.d().z(), 2.f),
    4.f * (ray.d().x() * ray.p().x() + ray.d().y() * ray.p().y() + ray.d().z() * ray.p().z()) * (ray.d().x() * ray.d().x() + ray.d().y() * ray.d().y() + ray.d().z() * ray.d().z()),
    2.f * (ray.d().x() * ray.d().x() + ray.d().y() * ray.d().y() + ray.d().z() * ray.d().z()) * (ray.p().x() * ray.p().x() + ray.p().y() * ray.p().y() + ray.p().z() * ray.p().z() + _r * _r - _h * _h) + 4.f * std::pow(ray.d().x() * ray.p().x() + ray.d().y() * ray.p().y() + ray.d().z() * ray.p().z(), 2.f) - 4.f * (ray.d().x() * ray.d().x() + ray.d().y() * ray.d().y()) * _r * _r,
    4.f * (ray.p().x() * ray.p().x() + ray.p().y() * ray.p().y() + ray.p().z() * ray.p().z() + _r * _r - _h * _h) * (ray.d().x() * ray.p().x() + ray.d().y() * ray.p().y() + ray.d().z() * ray.p().z()) - 8.f * (ray.d().x() * ray.p().x() + ray.d().y() * ray.p().y()) * _r * _r,
    std::pow(ray.p().x() * ray.p().x() + ray.p().y() * ray.p().y() + ray.p().z() * ray.p().z() + _r * _r - _h * _h, 2.f) - 4.f * (ray.p().x() * ray.p().x() + ray.p().y() * ray.p().y()) * _r * _r
    );

  // If torus overlap on himself, delete inside intersections
  if (_h > _r && result.size() == 4)
  {
    std::sort(result.begin(), result.end());
    if (std::abs(ray.p().z() + ray.d().z() * result[1]) < std::sqrt(1 - ((_r * _r) / (_h * _h))) * _h)
      return { result[0], result[3] };
  }

  return result;
}

Math::Vector<4>		RT::TorusCsgLeaf::normal(Math::Vector<4> const & pt) const
{
  // Point perfectly centered
  if (pt.x() == 0.f && pt.y() == 0.f)
    return Math::Vector<4>(0.f, 0.f, +1.f, 0.f);
  else
  {
    // Simple method to get n
    double	c = _r / std::sqrt(pt.x() * pt.x() + pt.y() * pt.y());

    if (std::abs(std::sqrt(std::pow(pt.x() - (pt.x() * c), 2) + std::pow(pt.y() - (pt.y() * c), 2) + std::pow(pt.z(), 2)) - _h) < Math::Shift)
      return Math::Vector<4>(pt.x() * (1.f - c), pt.y() * (1.f - c), pt.z(), 0.f);
    else
      return Math::Vector<4>(pt.x() * (1.f + c), pt.y() * (1.f + c), pt.z(), 0.f);
  }
}
