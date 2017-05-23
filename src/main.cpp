#include <iostream>
#include <string>
//boost headers
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/ply_io.h>
#include <pcl/io/pcd_io.h>

#include "defines.h"
#include "utils.h"
#include "operations.h"
#include <vector>
#include <string>
#include <algorithm>
//��ʼ��Utils��ľ�̬��Ա
bool Utils::pcd_ex = false;
bool Utils::ply_ex = false;



int main(){
	std::string pDirPath = "data";
	std::vector<std::string> pFileNames;
	Utils::get_clouds_filenames(pDirPath, pFileNames);
	bool use_pcd = Utils::pcd_ex;
	bool use_ply = Utils::ply_ex;
	std::cout << "use_pcd:" << use_pcd << std::endl;
	std::cout << "use_ply:" << use_ply << std::endl;

	std::vector<pcl::PointCloud<myPointT>::Ptr> cloudsBuffer;

	pcl::PointCloud<myPointT>::Ptr startCloud(new pcl::PointCloud<myPointT>);

	pcl::PointCloud<myPointT>::Ptr currentCloud(new pcl::PointCloud<myPointT>);
	pcl::PointCloud<myNormalT>::Ptr currentNormals(new pcl::PointCloud<myNormalT>);
	pcl::PointCloud<myPointT>::Ptr previousCloud(new pcl::PointCloud<myPointT>);
	pcl::PointCloud<myNormalT>::Ptr previousNormals(new pcl::PointCloud<myNormalT>);
	pcl::PointCloud<myNormalT>::Ptr normals(new pcl::PointCloud<myNormalT>);
	pcl::PointCloud<myPointT>::Ptr currentKeypoints(new pcl::PointCloud<myPointT>);
	pcl::PointCloud<myPointT>::Ptr previousKeypoints(new pcl::PointCloud<myPointT>);
	pcl::PointCloud<pcl::SHOT1344>::Ptr currentDescriptors(new pcl::PointCloud<pcl::SHOT1344>);
	pcl::PointCloud<pcl::SHOT1344>::Ptr previousDescriptors(new pcl::PointCloud<pcl::SHOT1344>);

	pcl::CorrespondencesPtr finalCorrespondences(new pcl::Correspondences);
//	pcl::PointCloud<myPointT>::Ptr currentInitAlinedCloud(new pcl::PointCloud<myPointT>);
	pcl::PointCloud<myPointT>::Ptr currentAlignedCloud(new pcl::PointCloud<myPointT>);

//	pcl::PointCloud<myPointT>::Ptr currentFinalAlinedCloud(new pcl::PointCloud<myPointT>);

	pcl::PointCloud<myPointT>::Ptr previousCloudTrans(new pcl::PointCloud<myPointT>);
	pcl::PointCloud<myPointT>::Ptr previousKeypointsTrans(new pcl::PointCloud<myPointT>);
	pcl::PointCloud<myPointT>::Ptr currentKeypointsTrans(new pcl::PointCloud<myPointT>);


	Eigen::Matrix4f initTransMatrix = Eigen::Matrix4f::Identity();
	Eigen::Matrix4f icpTransMatrix = Eigen::Matrix4f::Identity();
	Eigen::Matrix4f finalTransMatrix = Eigen::Matrix4f::Identity();
	Eigen::Matrix4f accTransMatrix = Eigen::Matrix4f::Identity();

	pcl::visualization::PCLVisualizer viewer("display");
	int vp1, vp2;
	viewer.createViewPort(0, 0, 0.5, 1, vp1);
	viewer.createViewPort(0.5, 0, 1, 1, vp2);
	//����
	int kNormalRadius = 10;

	std::stringstream frame_ID;

	int i = 0;
	pcl::PointCloud<myPointT>::Ptr concatenateCloud(new pcl::PointCloud<myPointT>);

	//�������е��ƣ����ν��д���
	for (auto it = pFileNames.begin(); it != pFileNames.end(); it++) {		
		
		pcl::PointCloud<myPointT>::Ptr tempCloud(new pcl::PointCloud<myPointT>);

		if (use_pcd) {
			pcl::io::loadPCDFile<myPointT>(*it, *currentCloud);
		}
		if (use_ply) {
			pcl::io::loadPLYFile<myPointT>(*it, *currentCloud);
		}



		//ÿ�����ƶ�Ҫȥ����Ⱥ��
		PointCloudOperations::ror_cloud(currentCloud, currentCloud, 10, 0.005);



		//��һ֡����׼��ֱ�Ӵ�����
		if (it == pFileNames.begin()) {

			pcl::copyPointCloud(*currentCloud, *currentAlignedCloud);
			pcl::copyPointCloud(*currentAlignedCloud, *tempCloud);

			cloudsBuffer.push_back(tempCloud);
			//�����ں�
			*concatenateCloud += *currentAlignedCloud;
			//��ǰ֡��Ϊǰһ֡			
			pcl::copyPointCloud(*currentCloud, *previousCloud);
//			std::cout << "size of the cloud: " << (*(cloudsBuffer.end() - 1))->size() << std::endl;// << "th cloud" << std::endl;
			continue;
		}

		//����ǰһ֡
//		pcl::copyPointCloud(**(cloudsBuffer.end()-1), *previousCloud);

		std::cout << "checkpoint" << std::endl;

		//����ǰһ֡�ķ���
		PointCloudOperations::compute_normals(previousCloud, 10, previousNormals);

		//����ǰһ֡susan�ؼ���
		PointCloudOperations::compute_susan(previousCloud, 0.005, previousKeypoints);

		//����ǰһ֡CSHOT
		PointCloudOperations::compute_cshot(previousCloud, previousNormals, previousKeypoints, 0.05, previousDescriptors);
		//PointCloudOperations::compute_cshot(previousCloud, previousNormals, previousCloud, 0.05, previousDescriptors);
		//���㵱ǰ֡�ķ���
		PointCloudOperations::compute_normals(currentCloud, 10, normals);

		//���㵱ǰ֡susan�ؼ���
		PointCloudOperations::compute_susan(currentCloud, 0.005, currentKeypoints);

		//����ǰ֡CSHOT
		PointCloudOperations::compute_cshot(currentCloud, normals, currentKeypoints, 0.05, currentDescriptors);
		//PointCloudOperations::compute_cshot(currentCloud, normals, currentCloud, 0.05, currentDescriptors);
		//����Ϊsource,ǰ��Ϊtarget
		//��ǰ��֡�����ӽ���ƥ��
		PointCloudOperations::find_correspondences(currentDescriptors, previousDescriptors, currentKeypoints, previousKeypoints, 0.005, finalCorrespondences);

		//����ǰ��֡�ĳ�ʼת�ƾ���
		PointCloudOperations::get_initTransMatrix(currentKeypoints, previousKeypoints, finalCorrespondences, initTransMatrix);

		//ǰ��֡���д���׼
		pcl::transformPointCloud(*currentCloud, *currentAlignedCloud, initTransMatrix);
		//pcl::copyPointCloud(*currentCloud, *currentAlignedCloud);
		//ǰ��֡����ICPϸ��׼			
		PointCloudOperations::icp_normals_align_cloud(currentAlignedCloud, previousCloud, currentAlignedCloud, 100, 1e-10, icpTransMatrix);
		//PointCloudOperations::icp_normals_align_cloud(currentAlignedCloud, previousCloud, currentAlignedCloud, 100, 1e-10, icpTransMatrix);
		//��׼�ĵ�ǰ֡�任����һ֡�ռ���
		pcl::transformPointCloud(*currentAlignedCloud, *currentAlignedCloud, accTransMatrix);

		//�����ں�
		*concatenateCloud += *currentAlignedCloud;
		
		//����׼�ĵ�ǰ֡������
		pcl::copyPointCloud(*currentAlignedCloud, *tempCloud);
		cloudsBuffer.push_back(tempCloud);

		//�����ۼƱ任����
		finalTransMatrix = icpTransMatrix*initTransMatrix;
		accTransMatrix = accTransMatrix*finalTransMatrix;

		//����ǰһ֡
		pcl::copyPointCloud(*currentCloud, *previousCloud);
	}

	std::cout << "size of cloudsbuffer: " << cloudsBuffer.size() << std::endl;
	//��һ֡�Ƶ�����λ��
	cloudsBuffer.push_back(*cloudsBuffer.begin());

//	cloudsBuffer.insert(cloudsBuffer.begin(), currentAlignedCloud);

	std::cout << "size of cloudsbuffer: " << cloudsBuffer.size() << std::endl;

	cloudsBuffer.erase(cloudsBuffer.begin());
//	cloudsBuffer.pop_back();
	std::reverse(cloudsBuffer.begin(), cloudsBuffer.end());
	std::cout << "size of cloudsbuffer: " << cloudsBuffer.size() << std::endl;
	int p = 0;


	initTransMatrix = Eigen::Matrix4f::Identity();
	icpTransMatrix = Eigen::Matrix4f::Identity();
	finalTransMatrix = Eigen::Matrix4f::Identity();
	accTransMatrix = Eigen::Matrix4f::Identity();

	viewer.addPointCloud(concatenateCloud, "1 fused cloud", vp1);

	currentCloud->clear();
	previousCloud->clear();
	concatenateCloud->clear();


	pcl::PointCloud<myPointT>::Ptr concatenateCloud2(new pcl::PointCloud<myPointT>);

//	viewer.addPointCloud(previousCloud, "previous cloud", vp1);

//	viewer.addPointCloud(currentCloud, "current cloud", vp1);
	//�ٴα������ƽ�����׼
	for (auto it = cloudsBuffer.begin(); it != cloudsBuffer.end(); it++) {


		std::cout << "cloud size: " << (*it)->size() << std::endl;

		frame_ID << p;
		p++;




		//���ص�ǰ֡
		pcl::copyPointCloud(**it, *currentCloud);


		//��һ֡����׼��ֱ�Ӵ�����
		if (it == cloudsBuffer.begin()) {

			pcl::copyPointCloud(*currentCloud, *currentAlignedCloud);
			//			pcl::copyPointCloud(*currentAlignedCloud, *tempCloud);

			//			cloudsBuffer.push_back(tempCloud);
			//�����ں�
			*concatenateCloud2 += *currentAlignedCloud;
			//��ǰ֡��Ϊǰһ֡			
			pcl::copyPointCloud(*currentCloud, *previousCloud);
			//			std::cout << "size of the cloud: " << (*(cloudsBuffer.end() - 1))->size() << std::endl;// << "th cloud" << std::endl;


//			viewer.addPointCloud(currentCloud, frame_ID.str(), vp1);

			continue;
		}

		//����ǰһ֡
		//		pcl::copyPointCloud(**(cloudsBuffer.end()-1), *previousCloud);

		std::cout << "checkpoint" << std::endl;

		//����ǰһ֡�ķ���
//		PointCloudOperations::compute_normals(previousCloud, 10, previousNormals);

		//����ǰһ֡susan�ؼ���
//		PointCloudOperations::compute_susan(previousCloud, 0.005, previousKeypoints);

		//����ǰһ֡CSHOT
//		PointCloudOperations::compute_cshot(previousCloud, previousNormals, previousKeypoints, 0.05, previousDescriptors);
///		PointCloudOperations::compute_cshot(previousCloud, previousNormals, previousCloud, 0.05, previousDescriptors);
		//���㵱ǰ֡�ķ���
//		PointCloudOperations::compute_normals(currentCloud, 10, normals);

		//���㵱ǰ֡susan�ؼ���
//		PointCloudOperations::compute_susan(currentCloud, 0.005, currentKeypoints);

		//����ǰ֡CSHOT
//		PointCloudOperations::compute_cshot(currentCloud, normals, currentKeypoints, 0.05, currentDescriptors);
	//	PointCloudOperations::compute_cshot(currentCloud, normals, currentCloud, 0.05, currentDescriptors);
		//����Ϊsource,ǰ��Ϊtarget
		//��ǰ��֡�����ӽ���ƥ��
		//PointCloudOperations::find_correspondences(currentDescriptors, previousDescriptors, currentKeypoints, previousKeypoints, 0.005, finalCorrespondences);
	//	PointCloudOperations::find_correspondences(currentDescriptors, previousDescriptors, currentCloud, previousCloud, 0.005, finalCorrespondences);
		//PointCloudOperations::find_correspondences(previousDescriptors, currentDescriptors,  previousKeypoints, currentKeypoints, 0.005, finalCorrespondences);
		//����ǰ��֡�ĳ�ʼת�ƾ���
	//	PointCloudOperations::get_initTransMatrix(currentCloud, previousCloud, finalCorrespondences, initTransMatrix);

		//ǰ��֡���д���׼
	//	pcl::transformPointCloud(*currentCloud, *currentAlignedCloud, initTransMatrix);
		pcl::copyPointCloud(*currentCloud, *currentAlignedCloud);
		//ǰ��֡����ICPϸ��׼			
		PointCloudOperations::icp_normals_align_cloud(currentAlignedCloud, previousCloud, currentAlignedCloud, 100, 1e-10, icpTransMatrix);

		//��׼�ĵ�ǰ֡�任����һ֡�ռ���
		pcl::transformPointCloud(*currentAlignedCloud, *currentAlignedCloud, accTransMatrix);
//		viewer.addPointCloud(currentCloud, frame_ID.str(), vp1);
		//�����ں�
		*concatenateCloud2 += *currentAlignedCloud;

		//����׼�ĵ�ǰ֡������
		//		pcl::copyPointCloud(*currentAlignedCloud, *tempCloud);
		//		cloudsBuffer.push_back(tempCloud);

		//�����ۼƱ任����
		finalTransMatrix = icpTransMatrix*initTransMatrix;
		accTransMatrix = accTransMatrix*finalTransMatrix;

		//����ǰһ֡
		pcl::copyPointCloud(*currentCloud, *previousCloud);

	}


	viewer.addPointCloud(concatenateCloud2, "2 fused cloud", vp2);


//	viewer.addPointCloud(previousCloud, "previous cloud", vp2);
//	pcl::visualization::PointCloudColorHandlerCustom<myPointT> keypoints_handler(currentKeypoints, 0, 0, 255);
//	viewer.addPointCloud(currentKeypoints, keypoints_handler, "currentkeypoints", vp1);
//	viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "currentkeypoints");
//	viewer.addPointCloud(previousKeypoints, keypoints_handler, "previouskeypoints", vp2);
//	viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "previouskeypoints");
	
//	pcl::visualization::PCLVisualizer viewer("display");
	//Ϊ����ͬһ��������ʾǰ��֡�����һ���ƥ���ߣ���Ҫ��ǰ֡ƽ��һ��
/*	pcl::transformPointCloud(*previousCloud, *previousCloudTrans, Eigen::Vector3f(-1, 0, 0), Eigen::Quaternionf(1, 0, 0, 0));
	pcl::transformPointCloud(*previousKeypoints, *previousKeypointsTrans, Eigen::Vector3f(-1, 0, 0), Eigen::Quaternionf(1, 0, 0, 0));

	std::cout << "previous keypoints: " << previousKeypointsTrans->size() << std::endl;

	std::cout << "current keypoints: " << currentKeypoints->size() << std::endl;

	std::cout << "corres: " << finalCorrespondences->size() << std::endl;

	viewer.addPointCloud(currentCloud, "current cloud2");
	viewer.addPointCloud(previousCloudTrans, "previous cloud");

	pcl::visualization::PointCloudColorHandlerCustom<myPointT> keypoints_handler(currentKeypoints, 0, 0, 255);

	viewer.addPointCloud(currentKeypoints, keypoints_handler, "currentkeypoints");
	viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "currentkeypoints");
	viewer.addPointCloud(previousKeypointsTrans, keypoints_handler, "previouskeypoints");
	viewer.setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "previouskeypoints");
	
	for (size_t i = 0; i < finalCorrespondences->size(); i++) {
		std::stringstream line_ID;
		line_ID << "correspondence_line_" << i;	
		viewer.addLine<myPointT, myPointT>(currentKeypoints->at(finalCorrespondences->at(i).index_match), previousKeypointsTrans->at(finalCorrespondences->at(i).index_query), 0, 255, 0, line_ID.str());
	}*/
	std::cout << "checkpoint" << std::endl;
	
	while (!viewer.wasStopped()) {
		viewer.spinOnce();
	}

	return 0;
	
}