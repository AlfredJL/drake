#pragma once

#include <stdint.h>
#include <memory>
#include <utility>

#include <Eigen/Dense>
#include <Eigen/StdVector>

#include "drake/drakeCollision_export.h"
#include "drake/systems/plants/shapes/DrakeShapes.h"

// Forward declaration.
// RigidBody's interface is never used by the collision engine.
// It is however useful for the concept of having a CollisionElement attached
// to a Body.
class RigidBody;

namespace DrakeCollision {
typedef uintptr_t CollisionElementId;

class DRAKECOLLISION_EXPORT CollisionElement : public DrakeShapes::Element {
 public:
  /** Constructs a new collision element with no geometry attached. **/
  // TODO(amcastro-tri): having an element without geometry does not seem right.
  CollisionElement(const Eigen::Isometry3d& T_element_to_local =
                       Eigen::Isometry3d::Identity());

  /** Constructs a new collision element with a given geometry and a local
  transform from the geometry's frame to the collision element's frame. **/
  CollisionElement(const DrakeShapes::Geometry& geometry,
                   const Eigen::Isometry3d& T_element_to_local =
                       Eigen::Isometry3d::Identity());

  /** Creates an exact copy of this collision element.
  The copy will point to the same rigid body as the original collision element.
  The copy is created in the heap and the caller responsible for the newly
  created resource. **/
  CollisionElement* clone() const override;

  /** The unique CollisionElementId for this collision element. **/
  CollisionElementId getId() const;

  // TODO(amcastro-tri): this is not overriden anyware and returns false always.
  // See issue #2481.
  virtual bool isStatic() const { return false; }

  /** Returns `true` if this element should be checked for collisions
  with the other object.  CanCollideWith should be symmetric: if
  A can collide with B, B can collide with A.
  Users should not call this method to test the collision of an element with
  itself, unless each collision element is guaranteed to belong to its own
  collision clique.
  This method does not support same-body checks and will incorrectly return true
  in that case. A proper broad phase should never test an element against
  itself. **/
  virtual bool CanCollideWith(const CollisionElement* other) const;

  /** Returns a pointer to the const RigidBody to which this CollisionElement
  is attached. **/
  // TODO(amcastro-tri): getBody() -> get_body()
  const RigidBody* const getBody() const;

  /** Set the rigid body this collision element is attached to. **/
  void set_rigid_body(const RigidBody* body);

  /** Adds this collision element to collision clique clique_id.

  CollisionElement's within a clique do not collide.
  Calling this method to add an element to a clique it already belongs to
  does not have any effect. **/
  void AddToCollisionClique(int clique_id);

  /** Returns the number of cliques this collision element belongs to. **/
  int number_of_cliques() const;

  /** Returns a reference to the list (an `std::vector`) of the collision
  cliques this collision element belongs to. **/
  const std::vector<int>& collision_cliques() const;

  /**
   * A toString method for this class.
   */
  friend DRAKECOLLISION_EXPORT std::ostream& operator<<(
      std::ostream&, const CollisionElement&);

 protected:
  CollisionElement(const CollisionElement& other);

 private:
  CollisionElementId id;
  const RigidBody* body_{};

  // Collision cliques are defined as a group of collision elements that do not
  // collide.
  // Collision cliques in Drake are represented simply by an integer.
  // A collision element can belong to more than one clique.
  // Conceptually it would seem like std::set is the right fit for
  // CollisionElement::collision_cliques_. However, std::set is really good for
  // dynamically adding elements to a set that needs to change.
  // Once you are done adding elements to your set, access time is poor when
  // compared to a simple std::vector (nothing faster than scanning a vector of
  // adjacent entries in memory).
  // Here adding elements to the cliques vector only happens during problem
  // setup by the user or from a URDF/SDF file. What we really want is that once
  // this vector is setup, we can query it very fast during simulation.
  // This is done in CollisionElement::CanCollideWith which to be Order(N)
  // requires the entries in CollisionElement::collision_cliques_ to be sorted.
  std::vector<int> collision_cliques_;

 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}  // namespace DrakeCollision
