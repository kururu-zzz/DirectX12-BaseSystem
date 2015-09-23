#include "Vertex.h"
#include "../../core/d3d.h"
#include <map>
#include <vector>
#include <algorithm>

namespace fbx
{
	namespace element
	{
		struct ModelBoneWeight
		{
			uint32_t boneIndex[4];
			DirectX::XMFLOAT4 boneWeight;
		};
		void CreateIndexList(const FbxMesh* fbxmesh, std::vector<int>* indexList)
		{
			auto polygonCount = fbxmesh->GetPolygonCount();

			indexList->reserve(polygonCount * 3);

			for (int i = 0; i < polygonCount; ++i) {
				indexList->emplace_back(fbxmesh->GetPolygonVertex(i, 0));
				indexList->emplace_back(fbxmesh->GetPolygonVertex(i, 1));
				indexList->emplace_back(fbxmesh->GetPolygonVertex(i, 2));
			}
		}

		void CreatePositionList(const FbxMesh* fbxmesh, const std::vector<int>* indexList, std::vector<DirectX::XMFLOAT3>* positionList)
		{
			positionList->reserve(indexList->size());

			for (auto index : *indexList) {
				auto controlPoint = fbxmesh->GetControlPointAt(index);
				positionList->emplace_back(DirectX::XMFLOAT3(static_cast<float>(controlPoint[0]), static_cast<float>(controlPoint[1]), static_cast<float>(controlPoint[2])));

				assert(static_cast<float>(controlPoint[3]) == 0.0);
			}
		}

		void CreateNormalList(const FbxMesh* fbxmesh, const std::vector<int>* indexList, std::vector<DirectX::XMFLOAT3>* normalList)
		{
			auto elementCount = fbxmesh->GetElementNormalCount();

			// 法線要素が1のみ対応
			assert(elementCount == 1);

			auto element = fbxmesh->GetElementNormal();
			auto mappingMode = element->GetMappingMode();
			auto referenceMode = element->GetReferenceMode();
			const auto& indexArray = element->GetIndexArray();
			const auto& directArray = element->GetDirectArray();

			// eDirctかeIndexDirectのみ対応
			assert((referenceMode == FbxGeometryElement::eDirect) || (referenceMode == FbxGeometryElement::eIndexToDirect));

			normalList->reserve(indexList->size());

			if (mappingMode == FbxGeometryElement::eByControlPoint)
			{
				// コントロールポイントでマッピング
				for (auto index : *indexList)
				{
					auto normalIndex = (referenceMode == FbxGeometryElement::eDirect)
						? index
						: indexArray.GetAt(index);
					auto normal = directArray.GetAt(normalIndex);
					normalList->push_back(
						DirectX::XMFLOAT3(
							static_cast<float>(normal[0]),
							static_cast<float>(normal[1]),
							static_cast<float>(normal[2])));

				}
			}
			else if (mappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				// ポリゴンバーテックス（インデックス）でマッピング
				auto indexByPolygonVertex = 0;
				auto polygonCount = fbxmesh->GetPolygonCount();
				for (int i = 0; i < polygonCount; ++i)
				{
					auto polygonSize = fbxmesh->GetPolygonSize(i);

					for (int j = 0; j < polygonSize; ++j)
					{
						auto normalIndex = (referenceMode == FbxGeometryElement::eDirect)
							? indexByPolygonVertex
							: indexArray.GetAt(indexByPolygonVertex);
						auto normal = directArray.GetAt(normalIndex);

						normalList->push_back(
							DirectX::XMFLOAT3(
								static_cast<float>(normal[0]),
								static_cast<float>(normal[1]),
								static_cast<float>(normal[2])));


						++indexByPolygonVertex;
					}

				}
			}
			else
			{
				// それ以外のマッピングモードには対応しない
				assert(false);
			}
		}

		void CreateUVList(const FbxMesh* fbxmesh, const std::vector<int>* indexList, int UVNum, std::vector<DirectX::XMFLOAT2>* uvList)
		{
			auto elementCount = fbxmesh->GetElementUVCount();
			if (UVNum + 1 > elementCount)
			{
				return;
			}

			auto element = fbxmesh->GetElementUV(UVNum);
			auto mappingMode = element->GetMappingMode();
			auto referenceMode = element->GetReferenceMode();
			const auto& indexArray = element->GetIndexArray();
			const auto& directArray = element->GetDirectArray();

			// eDirctかeIndexDirectのみ対応
			assert((referenceMode == FbxGeometryElement::eDirect) || (referenceMode == FbxGeometryElement::eIndexToDirect));

			uvList->reserve(indexList->size());

			if (mappingMode == FbxGeometryElement::eByControlPoint)
			{
				// コントロールポイントでマッピング
				for (auto index : *indexList)
				{
					auto uvIndex = (referenceMode == FbxGeometryElement::eDirect)
						? index
						: indexArray.GetAt(index);
					auto uv = directArray.GetAt(uvIndex);
					uvList->push_back(DirectX::XMFLOAT2(static_cast<float>(uv[0]), static_cast<float>(uv[1])));
				}
			}
			else if (mappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				// ポリゴンバーテックス（インデックス）でマッピング
				auto indexByPolygonVertex = 0;
				auto polygonCount = fbxmesh->GetPolygonCount();
				for (int i = 0; i < polygonCount; ++i)
				{
					auto polygonSize = fbxmesh->GetPolygonSize(i);

					for (int j = 0; j < polygonSize; ++j)
					{
						auto uvIndex = (referenceMode == FbxGeometryElement::eDirect)
							? indexByPolygonVertex
							: indexArray.GetAt(indexByPolygonVertex);
						auto uv = directArray.GetAt(uvIndex);

						uvList->push_back(DirectX::XMFLOAT2(static_cast<float>(uv[0]), static_cast<float>(uv[1])));

						++indexByPolygonVertex;
					}

				}
			}
			else
			{
				// それ以外のマッピングモードには対応しない
				assert(false);
			}
		}
		void CreateWeightList(const FbxMesh* fbxmesh, const std::vector<int>* indexList, std::vector<ModelBoneWeight>* boneWeightList)
		{
			auto skinCount = fbxmesh->GetDeformerCount(FbxDeformer::eSkin);
			if (skinCount == 0)
			{
				return;
			}

			// スキンが1しか対応しない
			assert(skinCount <= 1);

			auto controlPointsCount = fbxmesh->GetControlPointsCount();
			using TmpWeight = std::pair<int, float>;
			std::vector<std::vector<TmpWeight>> tmpBoneWeightList(controlPointsCount);

			auto skin = static_cast<FbxSkin*>(fbxmesh->GetDeformer(0, FbxDeformer::eSkin));
			auto clusterCount = skin->GetClusterCount();

			for (int i = 0; i < clusterCount; ++i)
			{
				auto cluster = skin->GetCluster(i);

				// eNormalizeしか対応しない
				assert(cluster->GetLinkMode() == FbxCluster::eNormalize);

				auto indexCount = cluster->GetControlPointIndicesCount();
				auto indices = cluster->GetControlPointIndices();
				auto weights = cluster->GetControlPointWeights();

				for (int j = 0; j < indexCount; ++j)
				{
					auto controlPointIndex = indices[j];
					tmpBoneWeightList[controlPointIndex].push_back({ i, static_cast<float>(weights[j]) });
				}
			}

			// コントロールポイントに対応したウェイトを作成
			std::vector<ModelBoneWeight> boneWeightListControlPoints;
			for (auto& tmpBoneWeight : tmpBoneWeightList)
			{
				if (tmpBoneWeight.size() > 0)
				{
					// ウェイトの大きさでソート
					std::sort(tmpBoneWeight.begin(), tmpBoneWeight.end(),
						[](const TmpWeight& weightA, const TmpWeight& weightB) { return weightA.second > weightB.second; }
						//[](const TmpWeight& weightA, const TmpWeight& weightB){ return weightA.second < weightB.second; }
					);

					// 1頂点に4つより多くウェイトが割り振られているなら影響が少ないものは無視する
					while (tmpBoneWeight.size() > 4)
					{
						tmpBoneWeight.pop_back();
					}

					// 4つに満たない場合はダミーを挿入
					while (tmpBoneWeight.size() < 4)
					{
						tmpBoneWeight.push_back({ 0, 0.0f });
					}

					ModelBoneWeight weight;
					float total = 0.0f;
					for (int i = 0; i < 4; ++i)
					{
						weight.boneIndex[i] = tmpBoneWeight[i].first;
						total += tmpBoneWeight[i].second;
					}
					weight.boneWeight.x = tmpBoneWeight[0].second / total;
					weight.boneWeight.y = tmpBoneWeight[1].second / total;
					weight.boneWeight.z = tmpBoneWeight[2].second / total;
					weight.boneWeight.w = tmpBoneWeight[3].second / total;

					boneWeightListControlPoints.push_back(weight);
				}
				else
				{
					ModelBoneWeight weight;
					for (auto i : { 0,1,2,3 })
						weight.boneIndex[i] = 0;
					weight.boneWeight = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f);
					boneWeightListControlPoints.push_back(weight);
				}
			}
			// インデックスで展開
			for (auto index : *indexList)
			{
				boneWeightList->push_back(boneWeightListControlPoints[index]);
			}
		}

		void CreateVertex(
			const FbxMesh* fbxmesh,
			D3D12_VERTEX_BUFFER_VIEW* vertexBufferView,
			D3D12_INDEX_BUFFER_VIEW* indexBufferView
			)
		{
			std::vector<int> indexList;
			CreateIndexList(fbxmesh, &indexList);

			std::vector<DirectX::XMFLOAT3> positionList;
			CreatePositionList(fbxmesh, &indexList, &positionList);

			std::vector<DirectX::XMFLOAT3> normalList;
			CreateNormalList(fbxmesh, &indexList, &normalList);

			std::vector<DirectX::XMFLOAT2> uv0List;
			CreateUVList(fbxmesh, &indexList, 0, &uv0List);

			std::vector<ModelBoneWeight> boneWeightList;
			CreateWeightList(fbxmesh, &indexList, &boneWeightList);

			assert(indexList.size() == positionList.size());
			assert(indexList.size() == normalList.size());

			std::vector<Vertex> vertexList;
			for (unsigned int i = 0; i < indexList.size(); ++i) {
				Vertex vertex;
				vertex.pos = positionList[i];
				vertex.normal = normalList[i];
				vertex.color = 0xffffffff;
				vertex.texel = (uv0List.size() == 0) ?	DirectX::XMFLOAT2(0.0f, 0.0f) :
														DirectX::XMFLOAT2(uv0List[i].x, -uv0List[i].y);

				if (boneWeightList.size() > 0) {
					for (int j = 0; j < 4; ++j)
						vertex.boneIndex[j] = boneWeightList[i].boneIndex[j];
					vertex.boneWeight = boneWeightList[i].boneWeight;
				}
				else
				{
					for (int j = 0; j < 4; ++j)
						vertex.boneIndex[j] = 0;
					vertex.boneWeight = DirectX::XMFLOAT4(0, 0, 0, 0);
				}

				vertexList.emplace_back(vertex);
			}

			std::vector<Vertex> modelVertexList;
			modelVertexList.reserve(vertexList.size());
			std::vector<int> modelIndexList;
			modelIndexList.reserve(indexList.size());

			std::map<Vertex, int> exist;
			for (auto& vertex : vertexList) {
				if (exist.count(vertex) == 0) {
					modelIndexList.emplace_back(modelVertexList.size());

					exist.emplace(vertex, modelVertexList.size());
					modelVertexList.emplace_back(vertex);
				}
				else {
					modelIndexList.emplace_back(exist[vertex]);
				}
			}
			auto vertexResource = d3d::CreateResource(sizeof(modelVertexList.data()));
			*vertexBufferView = d3d::CreateVertexBufferView(vertexResource.get(), modelVertexList.data(), sizeof(Vertex), modelVertexList.size());
			auto indexResource = d3d::CreateResource(sizeof(modelIndexList.data()));
			*indexBufferView = d3d::CreateIndexBufferView(indexResource.get(), modelIndexList.data() , sizeof(uint32_t), modelIndexList.size());
		}
	}
}