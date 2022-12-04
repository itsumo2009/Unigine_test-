#pragma once

#include "glm/vec3.hpp"

#include <vector>

class Path
{
public:
   class VelocityIterator
   {
   public:
      VelocityIterator(const Path& master, float velocity, float initialPosition);

      void advance(float deltaTime);

   public:
      glm::vec3 direction;
      glm::vec3 position;

   private:
      const Path& master_;
      const float velocity_;
      int currentSegment_;
      float segmentT_;
   };

   class IntervalIterator
   {
   public:
      IntervalIterator(const Path& master, float step);

      void advance();

   public:
      glm::vec3 direction;
      glm::vec3 position;
      glm::vec3 normal;

   private:
      const Path& master_;
      const float step_;
      int currentSegment_;
      float segmentT_;
   };
public:
   explicit Path(const std::vector<glm::vec3>& points);

   float length() const;
   const std::vector<glm::vec3>& segment_points() const;
private:
   void generateSegmentPoints(const std::vector<glm::vec3>& points);
   void generateSegmentParameters();

private:
   std::vector<glm::vec3> segment_points_;
   std::vector<glm::vec3> segment_directions_;
   std::vector<glm::vec3> segment_normals_;
   std::vector<float> segment_lengths_;
   float length_ = 0.0f;
};