#include <sstream>

#include "Math.hpp"
#include "OcclusionLight.hpp"
#include "Scene.hpp"

RT::OcclusionLight::OcclusionLight(RT::Color const & color, double radius, unsigned int quality)
  : _color(color), _radius(radius), _quality(quality)
{
  _radius = _radius > 0.f ? _radius : 0.f;
  _quality = _quality > 1 ? _quality : 1;
}

RT::OcclusionLight::~OcclusionLight()
{}

RT::Color RT::OcclusionLight::preview(RT::Scene const * scene, Math::Ray const & ray, Math::Ray const & normal, RT::Material const & material) const
{
  return material.color * material.ambient * _color * scene->config().lightAmbient;
}

RT::Color RT::OcclusionLight::render(RT::Scene const * scene, Math::Ray const & ray, Math::Ray const & normal, RT::Material const & material) const
{
  // If no ambient light, stop
  if (scene->config().lightAmbient == 0.f || material.ambient == 0.f || material.transparency == 1.f || material.reflection == 1.f)
    return RT::Color(0.f);

  // If quality to basic
  if (_quality <= 1 || _radius == 0.f)
    return material.color * scene->config().lightAmbient * material.ambient * (1.f - material.transparency) * (1.f - material.reflection);

  Math::Ray	n, occ;
  double	ry, rz;

  // Inverse normal if necessary
  n = normal;
  if (Math::Ray::cos(ray, normal) > 0)
    n.d() = Math::Matrix<4, 4>::scale(-1.f) * normal.d();

  // Calculate rotation angles of normal
  ry = -std::asin(n.dz());
  if (n.dx() != 0 || n.dy() != 0)
    rz = n.dy() > 0 ?
    +std::acos(n.dx() / std::sqrt(n.dx() * n.dx() + n.dy() * n.dy())) :
    -std::acos(n.dx() / std::sqrt(n.dx() * n.dx() + n.dy() * n.dy()));
  else
    rz = 0;

  // Rotation matrix to get ray to point of view
  Math::Matrix<4, 4>  matrix = Math::Matrix<4, 4>::rotation(0, Math::Utils::RadToDeg(ry), Math::Utils::RadToDeg(rz));

  // Point origin right above intersection
  occ.px() = n.px() + n.dx() * Math::Shift;
  occ.py() = n.py() + n.dy() * Math::Shift;
  occ.pz() = n.pz() + n.dz() * Math::Shift;

  // Generate and render occlusion rays
  unsigned int	nb_ray(0);
  RT::Color	ambient(0.f);

  for (double a = Math::Random::rand(Math::Pi / (2.f * _quality)); a < Math::Pi / 2.f; a += Math::Pi / (2.f * _quality))
    for (double b = Math::Random::rand(2.f * Math::Pi / (std::cos(a) * _quality * 2.f + 1.f)); b < 2.f * Math::Pi; b += (2.f * Math::Pi) / (std::cos(a) * _quality * 2.f + 1.f))
    {
      // Calculate ray according to point on the hemisphere
      occ.dx() = std::sin(a);
      occ.dy() = std::cos(b) * std::cos(a);
      occ.dz() = std::sin(b) * std::cos(a);
      occ.d() = matrix * occ.d();

      std::list<RT::Intersection> intersect = scene->tree()->render(occ.normalize());
      
      // Occlusion above
      if (material.transparency != 1.f)
      {
	std::list<RT::Intersection>::const_iterator it = intersect.begin();
	RT::Color				    light = RT::Color(_color);

	while (it != intersect.end() && +it->distance < 0.f)
	  it++;
	while (it != intersect.end() && +it->distance < _radius && light != RT::Color(0.f))
	{
	  // NOTE: occlusion ray is not reflected
	  ambient += RT::Color(+it->distance / _radius) * light * it->material.color * (1.f - it->material.transparency) * (1.f - material.transparency);
	  light *= it->material.color * it->material.transparency;
	  it++;
	}
	ambient += light * (1.f - material.transparency);
      }

      // Occlusion transparency
      if (material.transparency != 0.f)
      {
	std::list<RT::Intersection>::const_reverse_iterator it = intersect.rbegin();
	RT::Color					    light = RT::Color(_color);
	
	while (it != intersect.rend() && -it->distance < 0.f)
	  it++;
	while (it != intersect.rend() && light != RT::Color(0.f))
	{
	  // NOTE: occlusion ray is not reflected
	  ambient += RT::Color(-it->distance / _radius) * light * it->material.color * (1.f - it->material.transparency) * (material.transparency);
	  light *= it->material.color * it->material.transparency;
	  it++;
	}
	ambient += light * (material.transparency);
      }

      nb_ray++;
    }

  return ambient / nb_ray * scene->config().lightAmbient * material.color * material.ambient * (1.f - material.transparency) * (1.f - material.reflection);
}

std::string	RT::OcclusionLight::dump() const
{
  std::stringstream	stream;

  stream << "occlusion_light(" << _color.dump() << ", " << _radius << ", " << _quality << ");";

  return stream.str();
}