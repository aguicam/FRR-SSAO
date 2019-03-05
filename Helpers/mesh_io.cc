// Author: Marc Comino 2018

#include "./mesh_io.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "./triangle_mesh.h"

namespace data_representation {

namespace {

template <typename T>
void Add3Items(T i1, T i2, T i3, int index, std::vector<T> *vector) {
  (*vector)[index] = i1;
  (*vector)[index + 1] = i2;
  (*vector)[index + 2] = i3;
}

bool ReadPlyHeader(std::ifstream *fin, int *vertices, int *faces) {
  char line[100];

  fin->getline(line, 100);
  if (strncmp(line, "ply", 3) != 0) return false;

  *vertices = 0;
  fin->getline(line, 100);
  while (strncmp(line, "end_header", 10) != 0) {
    if (strncmp(line, "element vertex", 14) == 0) *vertices = atoi(&line[15]);
    fin->getline(line, 100);
    if (strncmp(line, "element face", 12) == 0) *faces = atoi(&line[13]);
  }

  if (*vertices <= 0) return false;

  std::cout << "Loading triangle mesh" << std::endl;
  std::cout << "\tVertices = " << *vertices << std::endl;
  std::cout << "\tFaces = " << *faces << std::endl;

  return true;
}

void ReadPlyVertices(std::ifstream *fin, TriangleMesh *mesh) {
  const int kVertices = mesh->vertices_.size() / 3;
  for (int i = 0; i < kVertices; ++i) {
    float x, y, z;
    fin->read(reinterpret_cast<char *>(&x), sizeof(float));
    fin->read(reinterpret_cast<char *>(&y), sizeof(float));
    fin->read(reinterpret_cast<char *>(&z), sizeof(float));

    Add3Items(x, y, z, i * 3, &(mesh->vertices_));
  }
}

void ReadPlyFaces(std::ifstream *fin, TriangleMesh *mesh) {
  unsigned char vertex_per_face;

  const int kFaces = mesh->faces_.size() / 3;
  for (int i = 0; i < kFaces; ++i) {
    int v1, v2, v3;
    fin->read(reinterpret_cast<char *>(&vertex_per_face),
              sizeof(unsigned char));
    assert(vertex_per_face == 3);

    fin->read(reinterpret_cast<char *>(&v1), sizeof(int));
    fin->read(reinterpret_cast<char *>(&v2), sizeof(int));
    fin->read(reinterpret_cast<char *>(&v3), sizeof(int));
    Add3Items(v1, v2, v3, i * 3, &(mesh->faces_));
  }
}

void ComputeVertexNormals(const std::vector<float> &vertices,
                          const std::vector<int> &faces,
                          std::vector<float> *normals) {
  const int kFaces = faces.size();
  std::vector<float> face_normals(kFaces, 0);

  for (int i = 0; i < kFaces; i += 3) {
    Eigen::Vector3d v1(vertices[faces[i] * 3], vertices[faces[i] * 3 + 1],
                       vertices[faces[i] * 3 + 2]);
    Eigen::Vector3d v2(vertices[faces[i + 1] * 3],
                       vertices[faces[i + 1] * 3 + 1],
                       vertices[faces[i + 1] * 3 + 2]);
    Eigen::Vector3d v3(vertices[faces[i + 2] * 3],
                       vertices[faces[i + 2] * 3 + 1],
                       vertices[faces[i + 2] * 3 + 2]);
    Eigen::Vector3d v1v2 = v2 - v1;
    Eigen::Vector3d v1v3 = v3 - v1;
    Eigen::Vector3d normal = v1v2.cross(v1v3);

    if (normal.norm() < 0.00001) {
      normal = Eigen::Vector3d(0.0, 0.0, 0.0);
    } else {
      normal.normalize();
    }

    for (int j = 0; j < 3; ++j) face_normals[i + j] = normal[j];
  }

  const int kVertices = vertices.size();
  normals->resize(kVertices, 0);
  for (int i = 0; i < kFaces; i += 3) {
    for (int j = 0; j < 3; ++j) {
      int idx = faces[i + j];
      Eigen::Vector3d v1(vertices[faces[i + j] * 3],
                         vertices[faces[i + j] * 3 + 1],
                         vertices[faces[i + j] * 3 + 2]);
      Eigen::Vector3d v2(vertices[faces[i + (j + 1) % 3] * 3],
                         vertices[faces[i + (j + 1) % 3] * 3 + 1],
                         vertices[faces[i + (j + 1) % 3] * 3 + 2]);
      Eigen::Vector3d v3(vertices[faces[i + (j + 2) % 3] * 3],
                         vertices[faces[i + (j + 2) % 3] * 3 + 1],
                         vertices[faces[i + (j + 2) % 3] * 3 + 2]);

      Eigen::Vector3d v1v2 = v2 - v1;
      Eigen::Vector3d v1v3 = v3 - v1;
      double angle = acos(v1v2.dot(v1v3) / (v1v2.norm() * v1v3.norm()));

      if (angle == angle) {
        for (int k = 0; k < 3; ++k) {
          (*normals)[idx * 3 + k] += face_normals[i + k] * angle;
        }
      }
    }
  }

  const int kNormals = normals->size();
  for (int i = 0; i < kNormals; i += 3) {
    Eigen::Vector3d normal((*normals)[i], (*normals)[i + 1], (*normals)[i + 2]);
    if (normal.norm() > 0.00001) {
      normal.normalize();
    } else {
      normal = Eigen::Vector3d(0, 0, 0);
    }

    for (int j = 0; j < 3; ++j) (*normals)[i + j] = normal[j];
  }
}

void ComputeBoundingBox(const std::vector<float> vertices, TriangleMesh *mesh) {
  const int kVertices = vertices.size() / 3;
  for (int i = 0; i < kVertices; ++i) {
    mesh->min_[0] = std::min(mesh->min_[0], vertices[i * 3]);
    mesh->min_[1] = std::min(mesh->min_[1], vertices[i * 3 + 1]);
    mesh->min_[2] = std::min(mesh->min_[2], vertices[i * 3 + 2]);

    mesh->max_[0] = std::max(mesh->max_[0], vertices[i * 3]);
    mesh->max_[1] = std::max(mesh->max_[1], vertices[i * 3 + 1]);
    mesh->max_[2] = std::max(mesh->max_[2], vertices[i * 3 + 2]);
  }
}

}  // namespace

bool ReadFromPly(const std::string &filename, TriangleMesh *mesh) {
  std::ifstream fin;

  fin.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
  if (!fin.is_open() || !fin.good()) return false;

  int vertices = 0, faces = 0;
  if (!ReadPlyHeader(&fin, &vertices, &faces)) {
    fin.close();
    return false;
  }

  mesh->vertices_.resize(vertices * 3);
  ReadPlyVertices(&fin, mesh);

  mesh->faces_.resize(faces * 3);
  ReadPlyFaces(&fin, mesh);

  fin.close();

  ComputeVertexNormals(mesh->vertices_, mesh->faces_, &mesh->normals_);
  ComputeBoundingBox(mesh->vertices_, mesh);

  return true;
}

bool WriteToPly(const std::string &filename, const TriangleMesh &mesh) {
  (void)filename;
  (void)mesh;

  std::cerr << "Not yet implemented" << std::endl;

  // TODO: Implement storing to PLY format.

  // END.

  return false;
}


void vertexCluster(int N,TriangleMesh *mesh){

    //Busquem la relació cell:{v}


    std::vector<std::vector<int>> cells;//vector contenint un vector per cada cell amb els index dels vertex
    std::vector<int> vec;
    for(int l=0;l<(N*N*N);l++){
        cells.push_back(vec);
    }
    //Posem l'index de cada vertex a la cel·la corresponent
    for(int v=0;v<mesh->vertices_.size()/3;v++){
        int index_cell = fmod((mesh->vertices_[3*v]-mesh->min_[0]),N)*pow(N,2)+fmod((mesh->vertices_[3*v+1]-mesh->min_[1]),N)*N+fmod((mesh->vertices_[3*v+2]-mesh->min_[2]),N);
        cells[index_cell].push_back(v);
    }

    //Creem els nous vertex
    std::vector<float> vertex_simp;

    for(int c=0;c<cells.size();c++){
        float vertex[3]={0,0,0};
        float count = 0.0;
        for(int v=0;v<cells[c].size();v++){
            count=count+1;
            vertex[0]=vertex[0]+mesh->vertices_[3*cells[c][v]];
            vertex[1]=vertex[1]+mesh->vertices_[3*cells[c][v]+1];
            vertex[2]=vertex[2]+mesh->vertices_[3*cells[c][v]+2];
        }
        vertex_simp.push_back(vertex[0]/count);
        vertex_simp.push_back(vertex[1]/count);
        vertex_simp.push_back(vertex[2]/count);
    }

    std::vector<std::vector<int>> rel_celdas;
    for(int l=0;l<cells.size();l++){
        rel_celdas.push_back(vec);
    }
//we need to check which cells are connected
    for(int f=0;f<mesh->faces_.size()/3;f++){
        int  index_0 = fmod((mesh->vertices_[3*mesh->faces_[3*f]]-mesh->min_[0]),N)*pow(N,2)+fmod((mesh->vertices_[3*mesh->faces_[3*f]+1]-mesh->min_[1]),N)*N+fmod((mesh->vertices_[3*mesh->faces_[3*f]+2]-mesh->min_[2]),N);
        int  index_1 = fmod((mesh->vertices_[3*mesh->faces_[3*f+1]]-mesh->min_[0]),N)*pow(N,2)+fmod((mesh->vertices_[3*mesh->faces_[3*f+1]+1]-mesh->min_[1]),N)*N+fmod((mesh->vertices_[3*mesh->faces_[3*f+1]+2]-mesh->min_[2]),N);
        int  index_2 = fmod((mesh->vertices_[3*mesh->faces_[3*f+2]]-mesh->min_[0]),N)*pow(N,2)+fmod((mesh->vertices_[3*mesh->faces_[3*f+2]+1]-mesh->min_[1]),N)*N+fmod((mesh->vertices_[3*mesh->faces_[3*f+2]+2]-mesh->min_[2]),N);
        if(index_0!=index_1){
            rel_celdas[index_0].push_back(index_1);
            rel_celdas[index_1].push_back(index_0);
        }
        else if (index_0!=index_2){
            rel_celdas[index_0].push_back(index_2);
            rel_celdas[index_2].push_back(index_0);
        }
        else if (index_1!=index_2){
            rel_celdas[index_1].push_back(index_2);
            rel_celdas[index_2].push_back(index_1);
        }
    }

   //we delete the the duplicates
    for(int l=0;l<rel_celdas.size();l++){
        std::sort(rel_celdas[l].begin(),rel_celdas[l].end());
        rel_celdas[l].erase(std::unique(rel_celdas[l].begin(),rel_celdas[l].end()),rel_celdas[l].end());
    }

}


//void TriangleMesh::octtree(int D){
//    //For D=2
//        std::vector<int> ind(vertices_.size()/3);
//        std::iota (std::begin(ind),std::end(ind),0);
//        std::vector<vector<int>> cells = subGroup(min_,max_,ind);

//        for(int l=0;l<cells.size();l++){


//        }
//}



//std::vector<std::vector<int>> subGroup(std::vector<float>min,std::vector<float> max,std::vector<int> vertex_in_cell,TriangleMesh *mesh){
//    std::vector<int> vec;
//    std::vector<std::vector<int>> cells={vec,vec,vec,vec,vec,vec,vec,vec};//cell with indexes to vertices

//    for(int v=0;v<vertex_in_cell.size();v++){
//        int i,j,k;
//        if(mesh->vertices_[3*vertex_in_cell[v]]<(max[0]+min[0])/2){
//            i=0;

//        }else{
//            i=1;
//        }
//        if(mesh->vertices_[3*vertex_in_cell[v]+1]<(max[1]+min[1])/2){
//            j=0;

//        }else{
//            j=1;
//        }
//        if(mesh->vertices_[3*vertex_in_cell[v]+2]<(max[2]+min[2])/2){
//            k=0;

//        }else{
//            k=1;
//        }
//        int index_cell= (i*4+2*j+k);//multiplicar por algo
//        cells[index_cell].push_back(3*vertex_in_cell[v]);
//        cells[index_cell].push_back(3*vertex_in_cell[v]+1);
//        cells[index_cell].push_back(3*vertex_in_cell[v]+2);
//    }
//    return cells;
//}



}  // namespace data_representation
