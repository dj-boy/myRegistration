#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <string>
// Boost headers
#include <boost/filesystem.hpp>

class Utils
{
public:
	//��̬��Ա������ʹ�õ�ʹ�ã�����ֱ�ӽ���Utils����namespace�����ض���һ��Utils���͵Ķ���
	//�÷���Utils::get_clouds_filenames(...)
	static bool pcd_ex;
	static bool ply_ex;
	static void get_clouds_filenames
		(
		const std::string & pDirectoryPath,
		std::vector<std::string> & pFilenames
		);

};

#endif