#include "utils.h"
//��ȡһ��Ŀ¼�����е����ļ���
//pDirectoryPath��·����, pFilenames�Ǹ�·���µ������ļ���
void Utils::get_clouds_filenames (const std::string & pDirectoryPath, std::vector<std::string> & pFilenames)
{
	//��stringת��Ϊboost�ļ�����
	boost::filesystem::path directory(pDirectoryPath);
	pFilenames.clear();
	//�ж�·���Ƿ���ڣ��Լ��Ƿ���Ŀ¼
	if (!boost::filesystem::exists(directory)||!boost::filesystem::is_directory(directory)){
		std::cerr << "invalid directory!" << std::endl;
	}
	std::vector<boost::filesystem::path> paths;
	//directory_iterator��Ŀ¼�������ļ����ļ��У��ĵ�����
	//��һ���ǵ���������㣨��һ���ļ������ڶ����ǵ��������յ㣨�޲�����
	std::copy(boost::filesystem::directory_iterator(directory), boost::filesystem::directory_iterator(), std::back_inserter(paths));
	std::sort(paths.begin(), paths.end());
	//const_iterator���ܸı���ָ���Ԫ��
	for (std::vector<boost::filesystem::path>::const_iterator it = paths.begin(); it != paths.end(); ++it)
	{
		if (it->extension().string() == ".pcd" || it->extension().string() == ".ply")
		{
			std::cout << *it << "\n";
			pFilenames.push_back(it->relative_path().string());
		}
	}
	pcd_ex = paths.begin()->extension().string() == ".pcd";
	ply_ex = paths.begin()->extension().string() == ".ply";

}