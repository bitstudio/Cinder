#include "algorithms/chromean.h"
#include <functional>
#include <list>

/*
 * DBSCAN(D, eps, MinPts) {
   C = 0
   for each point P in dataset D {
      if P is visited
         continue next point
      mark P as visited
      NeighborPts = regionQuery(P, eps)
      if sizeof(NeighborPts) < MinPts
         mark P as NOISE
      else {
         C = next cluster
         expandCluster(P, NeighborPts, C, eps, MinPts)
      }
   }
}

expandCluster(P, NeighborPts, C, eps, MinPts) {
   add P to cluster C
   for each point P' in NeighborPts {
      if P' is not visited {
         mark P' as visited
         NeighborPts' = regionQuery(P', eps)
         if sizeof(NeighborPts') >= MinPts
            NeighborPts = NeighborPts joined with NeighborPts'
      }
      if P' is not yet member of any cluster
         add P' to cluster C
   }
}

regionQuery(P, eps)
   return all points within P's eps-neighborhood (including P)
 *
 */


int Chromean::run(cv::Mat rgbMap, std::vector<cv::KeyPoint>& keypoints, cv::Mat descriptors)
{
	cv::Mat hsv;
	cv::Mat blurred;

	int blur_size = (rgbMap.cols / 128) * 2 + 1;
	cv::blur(rgbMap, blurred, cv::Size(blur_size, blur_size));
	cv::cvtColor(blurred, hsv, CV_BGR2HSV);
	
    std::vector<int> label(keypoints.size());
    std::fill(label.begin(), label.end(), -3);
    // -3 = unvisited
    // -3 != visited
    // -1 = noise

    int min_points = 10;
	double max_sqr_dist = dist_tolerance_*rgbMap.cols*dist_tolerance_*rgbMap.cols;

    auto region_query = [&](cv::KeyPoint p, std::list<int>& neighbors)
    {
        cv::Vec3b base = hsv.at<cv::Vec3b>(p.pt);
        for(int j = 0;j<keypoints.size();++j)
        {
            if(label[j] >= 0) continue;
            cv::Point2f d = p.pt - keypoints[j].pt;
            if(d.x*d.x + d.y*d.y > max_sqr_dist) continue;
            cv::Vec3b operand = hsv.at<cv::Vec3b>(keypoints[j].pt);
            double c_sqr_dist = 1.0*(base[0]-operand[0])*(base[0]-operand[0])
            + 0.1*(base[1]-operand[1])*(base[1]-operand[1])
            + 0.1*(base[2]-operand[2])*(base[2]-operand[2]);
            if(c_sqr_dist > 400) continue;
            neighbors.push_back(j);
        }
    };

    auto expand_cluster = [&](int pi, std::list<int>& neighbors, int cindex)
    {
        label[pi] = cindex;
        for(auto iter = neighbors.begin(); iter != neighbors.end(); ++iter)
        {
            if(label[*iter] == -3)
            {
                label[*iter] = -2;
                std::list<int> n;
                region_query(keypoints[*iter], n);
                if(n.size() >= min_points)
                {
                    neighbors.splice(neighbors.end(), n);
                }
            }
            if(label[*iter] < 0)
                label[*iter] = cindex;
        }

    };

    int C = 0;
    for(int i = 0;i<keypoints.size();++i)
    {
        if(label[i] != -3) continue;
        label[i] = -2;
        std::list<int> n;
        region_query(keypoints[i], n);
        if(n.size() < min_points)
        {
            label[i] = -1;
        }else{
            expand_cluster(i, n, C);
            C = C+1;
        }
    }

    clusters.resize(C);
    cluster_descriptors.resize(C);
    cluster_color_sum.resize(C);
    std::fill(cluster_color_sum.begin(), cluster_color_sum.end(), FColor());
    for(int i = 0;i<keypoints.size();++i)
    {
        if(label[i] >= 0)
        {
            clusters[label[i]].push_back(keypoints[i]);
            cluster_descriptors[label[i]].push_back(descriptors.row(i));
			cv::Vec3b operand = blurred.at<cv::Vec3b>(keypoints[i].pt);
            cluster_color_sum[label[i]] += operand;
        }
    }

    return C;
}
