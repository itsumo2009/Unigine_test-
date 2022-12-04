#include "Path.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
 

namespace
{
	glm::vec3 lerp(const glm::vec3& first, const glm::vec3& second, float t)
	{
		return first * (1 - t) + second * t;
	}
}

Path::Path(const std::vector<glm::vec3>& points)
{
	generateSegmentPoints(points);
	generateSegmentParameters();
}

float Path::length() const
{
	return length_;
}

const std::vector<glm::vec3>& Path::segment_points() const
{
	return segment_points_;
}

void Path::generateSegmentPoints(const std::vector<glm::vec3>& points)
{
	segment_points_.clear();
	const float dt = 0.01f;

	for (size_t i = 0; i < points.size(); ++i)
	{
		size_t j = i + 1;
		if (j == points.size())
		{
			j = 0;
		}

		size_t k = j + 1;
		if (k == points.size())
		{
			k = 0;
		}

		glm::vec3 centerFirst = 0.5f * (points[i] + points[j]);
		glm::vec3 centerSecond = 0.5f * (points[j] + points[k]);

		for (float t = 0; t < 1.0f; t += dt)
		{
			auto firstPoint = lerp(centerFirst, points[j], t);
			auto secondPoint = lerp(points[j], centerSecond, t);
			segment_points_.push_back(lerp(firstPoint, secondPoint, t));
		}
	}
}

void Path::generateSegmentParameters()
{
	for (size_t i = 0; i < segment_points_.size(); ++i)
	{
		size_t j = i + 1;
		if (j == segment_points_.size())
		{
			j = 0;
		}

		segment_directions_.push_back(glm::normalize(segment_points_[j] - segment_points_[i]));
		segment_normals_.push_back(glm::cross(glm::vec3(0, 1, 0), segment_directions_.back()));
		segment_lengths_.push_back(glm::distance(segment_points_[j], segment_points_[i]));
		length_ += segment_lengths_.back();
	}
	assert(length_ > 0);
}

Path::VelocityIterator::VelocityIterator(const Path& master, float velocity, float initialPosition)
	: master_(master)
	, velocity_(velocity)
{
	float position = initialPosition;
	while (position < 0)
	{
		position += master_.length();
	}

	float currentLength = 0.0f;
	for (int i = 0; i < master_.segment_lengths_.size(); ++i)
	{
		if (currentLength + master_.segment_lengths_[i] > position)
		{
			currentSegment_ = i;
			segmentT_ = (position - currentLength) / master_.segment_lengths_[i];
			int j = (i + 1) % master_.segment_lengths_.size();
			direction = lerp(master_.segment_directions_[i], master_.segment_directions_[j], segmentT_);
			this->position = lerp(master_.segment_points_[i], master_.segment_points_[j], segmentT_);
			return;
		}

		currentLength += master_.segment_lengths_[i];
	}
}

void Path::VelocityIterator::advance(float deltaTime)
{
	float shift = deltaTime * velocity_;

	float currentLocalPosition = master_.segment_lengths_[currentSegment_] * segmentT_;
	if (master_.segment_lengths_[currentSegment_] * (1 - segmentT_) > shift)
	{
		segmentT_ = (currentLocalPosition + shift) / master_.segment_lengths_[currentSegment_];
	}
	else
	{
		shift -= master_.segment_lengths_[currentSegment_] * (1 - segmentT_);
		++currentSegment_;

		if (currentSegment_ == master_.segment_lengths_.size())
		{
			currentSegment_ = 0;
		}

		while (master_.segment_lengths_[currentSegment_] < shift)
		{
			shift -= master_.segment_lengths_[currentSegment_];
			++currentSegment_;

			if (currentSegment_ == master_.segment_lengths_.size())
			{
				currentSegment_ = 0;
			}
		}
		segmentT_ = shift / master_.segment_lengths_[currentSegment_];
	}

	int j = (currentSegment_ + 1) % master_.segment_lengths_.size();
	direction = lerp(master_.segment_directions_[currentSegment_], master_.segment_directions_[j], segmentT_);
	position = lerp(master_.segment_points_[currentSegment_], master_.segment_points_[j], segmentT_);
}

Path::IntervalIterator::IntervalIterator(const Path& master, float step)
	: master_(master)
	, step_(step)
{
	currentSegment_ = 0;
	segmentT_ = 0;
	direction = master_.segment_directions_[0];
	position = master_.segment_points_[0];
	normal = master_.segment_normals_[0];
}

void Path::IntervalIterator::advance()
{
	float shift = step_;

	float currentLocalPosition = master_.segment_lengths_[currentSegment_] * segmentT_;
	if (master_.segment_lengths_[currentSegment_] * (1 - segmentT_) > shift)
	{
		segmentT_ = (currentLocalPosition + shift) / master_.segment_lengths_[currentSegment_];
	}
	else
	{
		shift -= master_.segment_lengths_[currentSegment_] * (1 - segmentT_);
		++currentSegment_;

		if (currentSegment_ == master_.segment_lengths_.size())
		{
			currentSegment_ = 0;
		}

		while (master_.segment_lengths_[currentSegment_] < shift)
		{
			shift -= master_.segment_lengths_[currentSegment_];
			++currentSegment_;

			if (currentSegment_ == master_.segment_lengths_.size())
			{
				currentSegment_ = 0;
			}
		}
		segmentT_ = shift / master_.segment_lengths_[currentSegment_];
	}

	int j = (currentSegment_ + 1) % master_.segment_lengths_.size();
	direction = lerp(master_.segment_directions_[currentSegment_], master_.segment_directions_[j], segmentT_);
	position = lerp(master_.segment_points_[currentSegment_], master_.segment_points_[j], segmentT_);
	normal = lerp(master_.segment_normals_[currentSegment_], master_.segment_normals_[j], segmentT_);
}
