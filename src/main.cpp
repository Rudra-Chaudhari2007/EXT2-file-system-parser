#include<iostream>
#include<bits/stdc++.h>
#include<cstring>
#include<fstream>
#include<cstdint>
#include<string>
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
    uint32_t inode_size;
};

struct Block_Group_Descriptor
{
    uint32_t a_block_usage_bitmap;    
    uint32_t a_inode_usage_bitmap;
    uint32_t a_inode_table;
    uint32_t n_unallocated_blocks;
    uint32_t n_unallocated_inodes;
    uint32_t n_directories;
    
};

struct Inode
{
    uint32_t pointer[15];
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

string name_converter(uint8_t* buffer, uint32_t n)
{
    string s = "";
    for(int i=0;i<n;i++)
    {
        s += (char)buffer[i];
    }
    return s;
}

Superblock read_superblock(Superblock sb , ifstream& file)
{
    file.seekg(1024);
    uint8_t buffer[100];
    file.read((char*)buffer,100);

        //file.read((char*)&sb , sizeof(sb));

        
        sb.inodes_count = converter(buffer + 0, 4);
        sb.blocks_count = converter(buffer + 4, 4);
        sb.r_blocks_count  = converter(buffer + 8, 4);
        sb.free_blocks_count = converter(buffer + 12, 4);
        sb.free_inodes_count = converter(buffer+ 16, 4);
        sb.first_data_block = converter(buffer + 20, 4);
        sb.log_block_size = converter(buffer +24, 4);
        sb.log_frag_size = converter(buffer + 28, 4);
        sb.blocks_per_group = converter(buffer+ 32, 4);
        sb.frags_per_group = converter(buffer + 36, 4);
        sb.inodes_per_group = converter(buffer+ 40, 4);
        sb.inode_size = converter(buffer +88, 2);
        
        return sb;
        
    }
    
    void print_superblock(Superblock sb)
{
    cout << "===== SUPERBLOCK =====" << endl;

    cout << "Inodes count      : " << sb.inodes_count << endl;
    cout << "Blocks count      : " << sb.blocks_count << endl;
    cout << "Reserved blocks   : " << sb.r_blocks_count << endl;
    cout << "Free blocks       : " << sb.free_blocks_count << endl;
    cout << "Free inodes       : " << sb.free_inodes_count << endl;
    cout << "First data block  : " << sb.first_data_block << endl;
    cout << "Block size        : " << (1024<<sb.log_block_size) << endl;
    cout << "Blocks per group  : " << sb.blocks_per_group << endl;
    cout << "Inodes per group  : " << sb.inodes_per_group << endl;
    cout << "Inode size        : " << sb.inode_size << endl;

    cout << endl;

}

Block_Group_Descriptor read_block_group_descriptor(Block_Group_Descriptor bgd, ifstream& file, int i)
{
    file.seekg(2048 + 32*i);

    uint8_t buffer[32];
    file.read((char*)buffer,32);

    bgd.a_block_usage_bitmap = converter(buffer + 0 , 4);
    bgd.a_inode_usage_bitmap = converter(buffer + 4 , 4);
    bgd.a_inode_table = converter(buffer + 8 , 4);
    bgd.n_unallocated_blocks = converter(buffer + 12 , 2);
    bgd.n_unallocated_inodes = converter(buffer + 14 , 2);
    bgd.n_directories = converter(buffer + 16 , 2);

    return bgd;
}

void print_block_group_descriptor(Block_Group_Descriptor bgd, ifstream& file, int i)
{
    cout << "=========Block" << i << "==========" << endl;

    cout << "Block address of block usage bitmap   : " << bgd.a_block_usage_bitmap << endl;
    cout << "Block address of inode usage bitmap   : " << bgd.a_inode_usage_bitmap << endl;
    cout << "Starting block address of inode table : " << bgd.a_inode_table << endl;
    cout << "Number of unallocated blocks in group : " << bgd.n_unallocated_blocks << endl;
    cout << "Number of unallocated inodes in group : " << bgd.n_unallocated_inodes << endl;
    cout << "Number of directories in group        : " << bgd.n_directories << endl;

    cout << endl;

}

void read_file(Superblock sb,Block_Group_Descriptor bgd,ifstream& file,uint32_t n)
{
    uint32_t block_size =1024 << sb.log_block_size;

    uint32_t block_group =(n - 1)/sb.inodes_per_group;

    uint32_t index =(n - 1) % sb.inodes_per_group;
    bgd = read_block_group_descriptor(bgd,file,block_group);

    uint64_t inode_position =(uint64_t)bgd.a_inode_table * block_size +(uint64_t)index * sb.inode_size;

    file.seekg(inode_position);

    uint8_t inode_buffer[sb.inode_size];

    file.read((char*)inode_buffer,sb.inode_size);

    uint32_t file_size =converter(inode_buffer + 4, 4);

    uint32_t pointer[15];

    for(int i = 0; i < 15; i++)
    {
        pointer[i] = converter(inode_buffer+ 40 +i*4,4);
    }

    uint32_t remaining = file_size;

    for(int i = 0; i < 12 && remaining > 0; i++)
    {
        if(pointer[i] == 0)
        continue;

        file.seekg((uint64_t)pointer[i] * block_size);

        uint32_t bytes_to_read =(remaining < block_size) ? remaining: block_size;

        uint8_t buffer[block_size];

        file.read((char*)buffer,bytes_to_read);

        for(uint32_t j = 0; j < bytes_to_read; j++)
        {
            cout << (char)buffer[j];
        }

        remaining -= bytes_to_read;
    }
}

void traverser(Superblock sb,Block_Group_Descriptor bgd,ifstream& file,uint32_t n,int depth);

void process_block(Superblock sb,Block_Group_Descriptor bgd,ifstream& file,uint32_t block,int depth)
{
    uint32_t block_size = 1024 << sb.log_block_size;

    file.seekg((uint64_t)block * block_size);

    uint8_t buffer[block_size];

    file.read((char*)buffer, block_size);

    uint32_t len = 0;

    while(len < block_size)
    {
        uint32_t entry_inode =converter(buffer+len, 4);
        uint32_t rec_len = converter(buffer + len + 4, 2);
        uint32_t name_len =converter(buffer + len + 6, 1);
        uint32_t file_type =converter(buffer + len + 7, 1);

        if(rec_len < 8)
        break;

        if(len + rec_len > block_size)
        break;

        string name = name_converter(buffer + len + 8, name_len);

        if(name == ".." || name == "."){
            len+=rec_len;
            continue;
        }

        if(entry_inode != 0)
        {
            for(int i = 0; i < depth; i++)
            cout << "    ";

            cout << "Name: " << name << endl;

            for(int i = 0; i < depth; i++)
            cout << "    ";
            cout << "Inode Number : " << entry_inode << endl;
            cout << endl;


            if(file_type == 2 && name != "." && name != "..")
            {
                //cout << "/" << endl;

                traverser(sb,bgd,file,entry_inode,depth + 1);
            }
            else
            {
                cout << endl;
            }
        }
        if(file_type == 1 && name == "readthis.txt")
        read_file(sb,bgd,file,entry_inode);

        len += rec_len;
    }

    
}

void process_indirect(Superblock sb,Block_Group_Descriptor bgd,ifstream& file,uint32_t block,int level,int depth)
{
    uint32_t block_size = 1024 << sb.log_block_size;

    file.seekg((uint64_t)block * block_size);

    uint32_t number_of_pointers = block_size / 4;

    uint8_t buffer[block_size];

    file.read((char*)buffer, block_size);

    for(uint32_t i = 0; i < number_of_pointers;i++)
    {
        uint32_t next_block = converter(buffer +i*4, 4);

        if(next_block == 0)
            continue;

        if(level == 1)
        {
            process_block(sb,bgd,file,next_block,depth);
        }
        else
        {
            process_indirect(sb,bgd,file,next_block,level - 1,depth);
        }
    }
}


void traverser(Superblock sb,Block_Group_Descriptor bgd,ifstream& file,uint32_t n,int depth)
{
    uint32_t block_group =(n - 1) / sb.inodes_per_group;
    uint32_t index =(n - 1) % sb.inodes_per_group;

    bgd = read_block_group_descriptor(bgd,file,block_group);

    uint32_t block_size =1024 << sb.log_block_size;

    uint64_t inode_position =(uint64_t)bgd.a_inode_table * block_size + (uint64_t)index * sb.inode_size;

    file.seekg(inode_position);
    uint8_t inode_buffer[sb.inode_size];

    file.read((char*)inode_buffer, sb.inode_size);

    uint32_t pointer[15];

    for(int i = 0; i < 15; i++)
    {
        pointer[i] = converter(inode_buffer + 40 + (i * 4),4);
    }

    for(int i = 0; i < 12; i++)
    {
        if(pointer[i] == 0)
        continue;

        process_block(sb,bgd,file,pointer[i],depth);
    }

    if(pointer[12] != 0)
    {
        process_indirect(sb,bgd,file,pointer[12],1,depth);
    }

    if(pointer[13] != 0)
    {
        process_indirect(sb,bgd,file,pointer[13],2,depth);
    }

    if(pointer[14] != 0)
    {
        process_indirect(sb,bgd,file,pointer[14],3,depth);
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

    
    Superblock sb;
    Block_Group_Descriptor bgd;

    sb = read_superblock(sb,file);
    uint32_t n = ceil(((float)sb.blocks_count)/(sb.blocks_per_group));

    string s = "";
    
    while("Aura")
    {
        cout << "prompt => " ;
        cin >> s;
        if(s == "superblock")
        print_superblock(sb);

        else if(s == "bgd")
        for(int i=0;i<n;i++)
        {
            bgd = read_block_group_descriptor(bgd,file,i);
            print_block_group_descriptor(bgd,file,i);
        }

        else if(s == "traverser")
        traverser(sb,bgd,file,2,0);

        else if(s == "exit")
        break;

        else
        {
            cout << "Command Not Found" << endl;
            cout << endl;
        }
    }

    return 0;
}
