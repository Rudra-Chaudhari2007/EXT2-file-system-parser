#include<iostream>
#include<bits/stdc++.h>
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

struct Block_Group_Descriptor_Table
{
    uint32_t a_block_usage_bitmap;    
    uint32_t a_inode_usage_bitmap;
    uint32_t a_inode_table;
    uint32_t n_unallocated_blocks;
    uint32_t n_unallocated_inodes;
    uint32_t n_directories;
    
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

uint32_t read_superblock(Superblock sb , ifstream& file)
{
    uint8_t buffer[44];
    file.read((char*)buffer,44);

        //file.read((char*)&sb , sizeof(sb));

    cout << "===== SUPERBLOCK =====" << endl;

    sb.inodes_count = converter(buffer + 0, 4);
    sb.blocks_count = converter(buffer + 4, 4);
    sb.r_blocks_count  = converter(buffer + 8, 4);
    sb.free_blocks_count = converter(buffer + 12, 4);
    sb.free_inodes_count = converter(buffer + 16, 4);
    sb.first_data_block = converter(buffer + 20, 4);
    sb.log_block_size = converter(buffer + 24, 4);
    sb.log_frag_size = converter(buffer + 28, 4);
    sb.blocks_per_group = converter(buffer + 32, 4);
    sb.frags_per_group = converter(buffer + 36, 4);
    sb.inodes_per_group = converter(buffer + 40, 4);

    cout << "Inodes count      : " << sb.inodes_count << endl;
    cout << "Blocks count      : " << sb.blocks_count << endl;
    cout << "Reserved blocks   : " << sb.r_blocks_count << endl;
    cout << "Free blocks       : " << sb.free_blocks_count << endl;
    cout << "Free inodes       : " << sb.free_inodes_count << endl;
    cout << "First data block  : " << sb.first_data_block << endl;
    cout << "Block size        : " << (1024<<sb.log_block_size) << endl;
    cout << "Blocks per group  : " << sb.blocks_per_group << endl;
    cout << "Inodes per group  : " << sb.inodes_per_group << endl;

    return ceil(((float)sb.blocks_count)/(sb.blocks_per_group));
}

void read_block_group_descriptor(Block_Group_Descriptor_Table bgdt, ifstream& file, int n)
{
    for(int i=0; i<n;i++)
    {
    file.seekg(2048 + 32*i);

    uint8_t buffer[32];
    file.read((char*)buffer,32);

    bgdt.a_block_usage_bitmap = converter(buffer + 0 , 4);
    bgdt.a_inode_usage_bitmap = converter(buffer + 4 , 4);
    bgdt.a_inode_table = converter(buffer + 8 , 4);
    bgdt.n_unallocated_blocks = converter(buffer + 12 , 2);
    bgdt.n_unallocated_inodes = converter(buffer + 14 , 2);
    bgdt.n_directories = converter(buffer + 16 , 2);


    cout << "=========Block" << i << "==========" << endl;

    cout << "Block address of block usage bitmap   : " << bgdt.a_block_usage_bitmap << endl;
    cout << "Block address of inode usage bitmap   : " << bgdt.a_inode_usage_bitmap << endl;
    cout << "Starting block address of inode table : " << bgdt.a_inode_table << endl;
    cout << "Number of unallocated blocks in group : " << bgdt.n_unallocated_blocks << endl;
    cout << "Number of unallocated inodes in group : " << bgdt.n_unallocated_inodes << endl;
    cout << "Number of directories in group        : " << bgdt.n_directories << endl;

    }

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
    Block_Group_Descriptor_Table bgdt;

    uint32_t n = read_superblock(sb,file);
    read_block_group_descriptor(bgdt,file,n);

    return 0;
}
