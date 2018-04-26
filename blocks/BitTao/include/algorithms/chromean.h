#ifndef CHROMEAN_H_
#define CHROMEAN_H_

#include <opencv2/opencv.hpp>
#include <iostream>

/**
 * @brief The Chromean class
 */

struct FColor
{
    float r, g, b;

    FColor()
    {
        r = 0;
        g = 0;
        b = 0;
    }

	FColor(const FColor & rhs)
	{
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
	}

	friend FColor operator/(const FColor& c0, const float operand) // copy assignment
    {
		FColor result;
		result.r = c0.r / operand;
		result.g = c0.g / operand;
		result.b = c0.b / operand;
        return result;
    }

    FColor& operator+=(const cv::Vec3b operand) // copy assignment
    {
        this->r = this->r + operand[0];
        this->g = this->g + operand[1];
        this->b = this->b + operand[2];
        return *this;
    }

	friend float operator-(const FColor& c0, const FColor& c1) // copy assignment
	{	
		float sum = 0;
		sum += std::abs(c0.r - c1.r);
		sum += std::abs(c0.g - c1.g);
		sum += std::abs(c0.b - c1.b);
		return sum;
	}
};

class Chromean
{
public:

	Chromean(float dist_tolerance = 0.1)
	{
		dist_tolerance_ = dist_tolerance;
	}

    int run(cv::Mat rgbMap, std::vector<cv::KeyPoint>& keypoints, cv::Mat descriptors);
    int get_cluster(int index, std::vector<cv::KeyPoint>& cluster, cv::Mat& descriptors, FColor& color)
    {
        cluster = clusters[index];
        descriptors = cluster_descriptors[index];
        color = cluster_color_sum[index]/cluster.size();
        return cluster.size();
    }

private:
    std::vector<std::vector<cv::KeyPoint>> clusters;
    std::vector<cv::Mat> cluster_descriptors;
    std::vector<FColor> cluster_color_sum;
	float dist_tolerance_;

};

#endif
