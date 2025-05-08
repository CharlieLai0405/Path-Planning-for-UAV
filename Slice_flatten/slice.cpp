#include <iostream>
#include <string>
#include <vector>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/common/common.h>

using namespace pcl;
using namespace std;

void slicePointCloud(const string& input_file, const string& output_file_prefix, float threshold) {
    // ���J�I��
    PointCloud<PointXYZ>::Ptr cloud(new PointCloud<PointXYZ>);
    if (io::loadPCDFile<PointXYZ>(input_file, *cloud) == -1) {
        PCL_ERROR("Couldn't read the pcd file\n");
        return;
    }

    // ���̤p�M�̤j Z ��
    float min_z = cloud->points[0].z;
    float max_z = cloud->points[0].z;
    for (const auto& point : cloud->points) {
        if (point.z < min_z) min_z = point.z;
        if (point.z > max_z) max_z = point.z;
    }

    cout << "Minimum Z value: " << min_z << endl;
    cout << "Maximum Z value: " << max_z << endl;

    // �ھ� threshold ���� Z �d��
    int slice_index = 0;
    float slice_start = min_z;
    float slice_end = slice_start + threshold;

    // �T�O���νd�򨬰�
    while (slice_start < max_z) {
        // �L�o�X Z �Ȥ��� slice_start �M slice_end �������I
        PointIndices::Ptr indices(new PointIndices);
        for (size_t i = 0; i < cloud->points.size(); ++i) {
            if (cloud->points[i].z >= slice_start && cloud->points[i].z < slice_end) {
                indices->indices.push_back(i);
            }
        }

        // �p�G�Ӽh����ơA�h�s�x�Ӽh���I��
        if (!indices->indices.empty()) {
            ExtractIndices<PointXYZ> extract;
            extract.setInputCloud(cloud);
            extract.setIndices(indices);
            extract.setNegative(false); // �O�d�o���I

            PointCloud<PointXYZ>::Ptr sliced_cloud(new PointCloud<PointXYZ>);
            extract.filter(*sliced_cloud);

            // �N�Ӽh�I���� Z �y�г]���d�򤤶���
            float mid_z = (slice_start + slice_end) / 2.0f;
            for (auto& point : sliced_cloud->points) {
                point.z = mid_z; // �ק� Z �y�Ь�������
            }

            // �O�s���Ϊ��I��
            string output_file = output_file_prefix + to_string(slice_index) + ".pcd";
            io::savePCDFileASCII(output_file, *sliced_cloud);
            cout << "Saved sliced point cloud with modified Z: " << output_file << endl;
        }

        // ��s Z �Ƚd��
        slice_start = slice_end;
        slice_end = slice_start + threshold;
        ++slice_index;
    }

    cout << "Slicing completed!" << endl;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <input.pcd> <output_prefix> <threshold>" << endl;
        return -1;
    }

    string input_file = argv[1];
    string output_file_prefix = argv[2];
    float threshold = stof(argv[3]);

    slicePointCloud(input_file, output_file_prefix, threshold);

    return 0;
}
