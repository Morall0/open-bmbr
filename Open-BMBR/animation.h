#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "bone.h"
#include <functional>
#include "animdata.h"
#include "model_animation.h"

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class Animation
{
public:
	Animation() = default;

	Animation(const std::string& animationPath, Model* model, unsigned int animationIndex)
	{
		Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath,
        aiProcess_Triangulate |
        aiProcess_PopulateArmatureData);
    // std::cout << "Scene loaded" << std::endl;
		assert(scene && scene->mRootNode);
    // std::cout << "Num animations: " << scene->mNumAnimations << std::endl;
    // if (scene->mNumAnimations == 0) {
    //   std::cout << "ERROR: 0 animations" << std::endl;
    //   return;
    // }
		auto animation = scene->mAnimations[animationIndex]; // Select the desired anim
    // std::cout << "Animation name: " << animation->mName.C_Str() << std::endl;
    // for (unsigned int i = 0; i<scene->mNumAnimations; i++)
    //   std::cout << "Animation name [" << i << "]: " << scene->mAnimations[i]->mName.C_Str() << std::endl;
		m_Duration = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;

    // std::cout << "Reading Hierarchy..." << std::endl;
    aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
    globalTransformation = globalTransformation.Inverse();
    m_GlobalInverseTransform = AssimpGLMHelpers::ConvertMatrixToGLMFormat(globalTransformation);
		ReadHierarchyData(m_RootNode, scene->mRootNode);
    // std::cout << "Reading bones..." << std::endl;
		ReadMissingBones(animation, *model);
	}

	~Animation()
	{
	}

	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	
	inline float GetTicksPerSecond() { return m_TicksPerSecond; }
	inline float GetDuration() { return m_Duration;}
	inline const AssimpNodeData& GetRootNode() { return m_RootNode; } inline const std::map<std::string,BoneInfo>& GetBoneIDMap()
	{ 
		return m_BoneInfoMap;
	}
  inline const glm::mat4& GetGlobalInverseTransform() { return m_GlobalInverseTransform; }
private:
	void ReadMissingBones(const aiAnimation* animation, Model& model)
	{
		int size = animation->mNumChannels;

		auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				boneInfoMap[channel->mNodeName.data].id, channel));
		}

		m_BoneInfoMap = boneInfoMap;
	}

	void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (unsigned int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHierarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}
	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
  glm::mat4 m_GlobalInverseTransform;
};
