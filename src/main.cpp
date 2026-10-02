#include<iostream>
#include<cstring>
#include<fstream>
#include<cstdint>
using namespace std;

struct Superblock
{
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
};

uint32_t converter(uint8_t* buffer, int n)
{
    uint32_t sum =0;
    for(int i=0;i<n;i++)
    {
        sum += ((uint32_t)buffer[i] << 8*i);
    }
    return sum;
}

void read_superblock(Superblock sb , ifstream& file)
{
    uint8_t buffer[44];
    file.read((char*)buffer,44);

        //file.read((char*)&sb , sizeof(sb));

    cout << "===== SUPERBLOCK =====" << endl;

    sb.inodes_count      = converter(buffer + 0, 4);
    sb.blocks_count      = converter(buffer + 4, 4);
    sb.r_blocks_count    = converter(buffer + 8, 4);
    sb.free_blocks_count = converter(buffer + 12, 4);
    sb.free_inodes_count = converter(buffer + 16, 4);
    sb.first_data_block  = converter(buffer + 20, 4);
    sb.log_block_size    = converter(buffer + 24, 4);
    sb.log_frag_size     = converter(buffer + 28, 4);
    sb.blocks_per_group  = converter(buffer + 32, 4);
    sb.frags_per_group   = converter(buffer + 36, 4);
    sb.inodes_per_group  = converter(buffer + 40, 4);

    cout << "Inodes count      : " << sb.inodes_count << endl;
    cout << "Blocks count      : " << sb.blocks_count << endl;
    cout << "Reserved blocks   : " << sb.r_blocks_count << endl;
    cout << "Free blocks       : " << sb.free_blocks_count << endl;
    cout << "Free inodes       : " << sb.free_inodes_count << endl;
    cout << "First data block  : " << sb.first_data_block << endl;
    cout << "Block size        : " << (1024<<sb.log_block_size) << endl;
    cout << "Blocks per group  : " << sb.blocks_per_group << endl;
    cout << "Inodes per group  : " << sb.inodes_per_group << endl;
}


int main()
{
    ifstream file ("/home/rudra/projects/disk-backpup.img" , ios :: binary);

    if(!file)
    {
        cout << "Couldn't open file!!" << endl;
        return 1;
    }

    file.seekg(1024);
    Superblock sb;

    read_superblock(sb,file);

    return 0;
}
