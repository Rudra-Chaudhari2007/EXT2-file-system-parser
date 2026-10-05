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

void converter(uint8_t* buffer, uint32_t value, int n)
{
    for(int i = 0; i < n; i++)
    {
        buffer[i] = value & 255;
        value = value >> 8;
    }
}

Superblock read_superblock(Superblock sb , fstream& file)
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

Block_Group_Descriptor read_block_group_descriptor(Block_Group_Descriptor bgd, fstream& file, int i)
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

void print_block_group_descriptor(Block_Group_Descriptor bgd, fstream& file, int i)
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

void read_file(Superblock sb,Block_Group_Descriptor bgd,fstream& file,uint32_t n)
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

void traverser(Superblock sb,Block_Group_Descriptor bgd,fstream& file,uint32_t n,int depth);

void process_block(Superblock sb,Block_Group_Descriptor bgd,fstream& file,uint32_t block,int depth)
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

void process_indirect(Superblock sb,Block_Group_Descriptor bgd,fstream& file,uint32_t block,int level,int depth)
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


void traverser(Superblock sb,Block_Group_Descriptor bgd,fstream& file,uint32_t n,int depth)
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

uint32_t find_file(Superblock sb,Block_Group_Descriptor bgd,fstream& file,uint32_t n,string name)
{
    uint32_t block_group =(n - 1) / sb.inodes_per_group;
    uint32_t index =(n - 1) % sb.inodes_per_group;

    bgd = read_block_group_descriptor(bgd,file,block_group);

    uint32_t block_size =1024 << sb.log_block_size;

    uint64_t inode_position =(uint64_t)bgd.a_inode_table * block_size + (uint64_t)index * sb.inode_size;
    file.seekg(inode_position);

    uint8_t inode_buffer[sb.inode_size];
    file.read((char*)inode_buffer,sb.inode_size);

    uint32_t pointer[15];

    for(int i = 0; i < 15; i++)
    {
        pointer[i] = converter(inode_buffer + 40 + (i*4),4);
    }

    // Check direct blocks
    for(int i = 0; i < 12; i++)
    {
        if(pointer[i] == 0)
            continue;

        uint64_t block_position =(uint64_t)pointer[i] * block_size;

        file.seekg(block_position);
        uint8_t buffer[block_size];

        file.read((char*)buffer,block_size);

        uint32_t len = 0;

        while(len < block_size)
        {
            uint32_t entry_inode =converter(buffer + len, 4);
            uint32_t rec_len = converter(buffer + len + 4, 2);
            uint32_t name_len = converter(buffer + len + 6, 1);

            if(rec_len < 8)
            break;

            if(len + rec_len > block_size)
            break;

            if(entry_inode != 0)
            {
                string entry_name =name_converter(buffer + len + 8,name_len);

                if(entry_name == name)
                {
                    return entry_inode;
                }
            }

            len += rec_len;
        }
    }

    return 0;
}

uint32_t change_directory(Superblock sb,Block_Group_Descriptor bgd,fstream& file,uint32_t n,string name)
{
    uint32_t block_group =(n-1) / sb.inodes_per_group;
    uint32_t index =(n- 1) % sb.inodes_per_group;

    bgd = read_block_group_descriptor(bgd,file,block_group);
    uint32_t block_size =1024 << sb.log_block_size;

    uint64_t inode_position =(uint64_t)bgd.a_inode_table * block_size +(uint64_t)index * sb.inode_size;

    file.seekg(inode_position);

    uint8_t inode_buffer[sb.inode_size];
    file.read((char*)inode_buffer, sb.inode_size);

    uint32_t pointer[15];
    for(int i = 0; i < 15; i++)
    {
        pointer[i] =converter(inode_buffer + 40 +(i*4),4);
    }

    // Check directory blocks
    for(int i = 0; i < 12; i++)
    {
        if(pointer[i] == 0)
        continue;

        file.seekg((uint64_t)pointer[i] * block_size);

        uint8_t buffer[block_size];
        file.read((char*)buffer,block_size);

        uint32_t len = 0;

        while(len < block_size)
        {
            uint32_t entry_inode =converter(buffer + len, 4);
            uint32_t rec_len =converter(buffer + len + 4, 2);
            uint32_t name_len =converter(buffer + len + 6, 1);
            uint32_t file_type =converter(buffer + len + 7, 1);

            if(rec_len < 8 || len + rec_len > block_size)
            break;

            if(entry_inode != 0)
            {
                string entry_name = name_converter(buffer+ len + 8, name_len);

                if(entry_name == name)
                {
                    if(file_type == 2)
                    {
                        cout << "changed directory to: " << name << endl;
                        return entry_inode;
                    }
                    else
                    {
                        cout << name << " is not a directory!" << endl;
                        return 0;
                    }
                }
            }
            len += rec_len;
        }
    }

    cout << "Directory not found!" << endl;

    return 0;
}


//1
uint32_t find_free_block(Superblock sb,Block_Group_Descriptor bgd,fstream& file,uint32_t block_group)
{
    uint32_t block_size =1024 << sb.log_block_size;
  
    bgd = read_block_group_descriptor(bgd, file, block_group);
    file.seekg((uint64_t)bgd.a_block_usage_bitmap * block_size);

    uint8_t buffer[block_size];

    file.read((char*)buffer,block_size);

    // First block belonging to this group
    uint32_t first_block =sb.first_data_block +block_group * sb.blocks_per_group;

    // Check every bit in the bitmap
    for(uint32_t i = 0; i < sb.blocks_per_group; i++)
    {
        uint32_t byte = i / 8;
        uint32_t bit = i % 8;

        // Bit is 0 -> block is free
        if((buffer[byte] & (1 << bit)) == 0)
        {
            uint32_t actual_block =first_block + i;

            return actual_block;
        }
    }

    return 0;
}

uint32_t allocate_block(Superblock& sb,Block_Group_Descriptor& bgd,fstream& file,uint32_t block_group)
{
    uint32_t block_size =1024 << sb.log_block_size;
    uint32_t block = find_free_block(sb,bgd,file,block_group);

    if(block == 0)
    {
        return 0;
    }

    uint32_t first_block =sb.first_data_block + block_group * sb.blocks_per_group;

    uint32_t i =block - first_block;
    uint32_t byte = i/8;
    uint32_t bit = i%8;

    file.seekg((uint64_t)bgd.a_block_usage_bitmap * block_size);

    uint8_t buffer[block_size];
    file.read((char*)buffer,block_size);

    buffer[byte] |= (1 << bit);

    file.seekp((uint64_t)bgd.a_block_usage_bitmap * block_size);

    file.write((char*)buffer,block_size);

    bgd.n_unallocated_blocks--;
    sb.free_blocks_count--;

    file.seekp(2048 + 32 * block_group + 12);

    uint8_t bgd_buffer[2];

    converter(bgd_buffer,bgd.n_unallocated_blocks,2);

    file.write((char*)bgd_buffer,2);

    file.seekp(1024 + 12);

    uint8_t sb_buffer[4];
    converter(sb_buffer,sb.free_blocks_count,4);

    file.write((char*)sb_buffer,4);

    return block;
}

void update_inode_size(fstream& file,uint64_t inode_position,uint32_t new_size)
{
    uint8_t buffer[4];
    converter(buffer, new_size, 4);
    file.seekp(inode_position + 4);

    file.write((char*)buffer,4);
}

void update_file(Superblock& sb,Block_Group_Descriptor bgd,fstream& file,uint32_t inode_number,string data,bool append)
{
    uint32_t block_group = (inode_number - 1) / sb.inodes_per_group;
    uint32_t index =(inode_number - 1) % sb.inodes_per_group;

    bgd = read_block_group_descriptor(bgd,file,block_group);

    uint32_t block_size =1024 << sb.log_block_size;
    uint64_t inode_position =(uint64_t)bgd.a_inode_table * block_size + (uint64_t)index * sb.inode_size;

    file.seekg(inode_position);

    uint8_t inode_buffer[sb.inode_size];
    file.read((char*)inode_buffer,sb.inode_size);

    uint32_t old_size = converter(inode_buffer + 4,4);

    uint32_t pointer[15];
    for(int i=0;i<15;i++)
    {
        pointer[i] =converter(inode_buffer + 40 +(i*4), 4);
    }

    uint32_t start_position;
    uint32_t new_size;

    if(append)
    {
        start_position = old_size;

        if(old_size > 0)
        {
            file.seekg(
                (uint64_t)pointer[0] * block_size + old_size - 1
            );

            char last_character;

            file.read(&last_character, 1);

            if(last_character == '\n')
            {
                start_position = old_size - 1;
                new_size = old_size - 1 + data.size();
            }
            else
            {
                new_size = old_size + data.size();
            }
        }
        else
        {
            new_size = data.size();
        }
    }
    else
    {
        start_position = 0;
        new_size = data.size();
    }

    uint32_t required_blocks = (new_size + block_size - 1) / block_size;
    uint32_t existing_blocks = 0;

    for(int i=0;i<12;i++)
    {
        if(pointer[i] != 0)
        existing_blocks++;
    }

    for(int i = existing_blocks; i < required_blocks;i++)
    {
        uint32_t new_block =allocate_block(sb,bgd,file,block_group);

        if(new_block == 0)
        {
            cout << "No free blocks available!" << endl;
            return;
        }

        pointer[i] = new_block;
        file.seekp(inode_position + 40 + (i * 4));

        uint8_t buffer[4];
        converter(buffer,new_block,4);

        file.write((char*)buffer,4);
    }

    uint32_t position = start_position;
    uint32_t data_position = 0;
    uint32_t remaining = data.size();

    while(remaining > 0)
    {
        uint32_t block_number = position/block_size;

        uint32_t offset =position % block_size;

        uint32_t bytes_to_write = block_size - offset;

        if(bytes_to_write > remaining)
        bytes_to_write = remaining;

        uint32_t actual_block =
        pointer[block_number];

        file.seekp((uint64_t)actual_block * block_size+ offset);

        file.write(data.data() + data_position,bytes_to_write);

        position += bytes_to_write;
        data_position += bytes_to_write;
        remaining -= bytes_to_write;
    }

    update_inode_size(file,inode_position,new_size);
}

int main()
{
    fstream file ("/home/rudra/projects/disk-backpup.img" , ios :: in | ios :: out | ios :: binary);

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
    
    uint32_t current_dir =2;
    while("WEC")
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

        else if(s == "cd")
        {
            string r;
            cin >> r;
            uint32_t a = change_directory(sb,bgd,file,current_dir,r);
            if(a)
            {
                current_dir = a;
            }
            else
            continue;
        }

        else if(s == "update")
        {
            cout << "Enter Filename: ";
            string fn;
            cin >> fn; cout << endl;
            uint32_t inode_number = find_file(sb,bgd,file,current_dir,fn);
            if(!inode_number)
            {
                cout << "File Not Found" << endl;
                continue;
            }
            int choice;

            cout << "1. Overwrite" << endl;
            cout << "2. Append" << endl;
            cout << "Enter choice: ";

            cin >> choice;

            if(choice >2)
            {
                cout << "Invalid Choice" << endl;
                cout << endl;
                continue;
            }
            string data;

            cout << "Enter data: ";
            cin.ignore();
            getline(cin, data);
            data += '\n';

            //if(inode_number != 0)

            if(choice == 1)
            {
                update_file(sb,bgd,file,inode_number,data,false);
            }
            else if(choice == 2)
            {
                update_file(sb,bgd,file,inode_number,data,true);
            }

        }

        else if(s == "read")
        {
            string fn;
            cin >> fn;

            uint32_t a = find_file(sb,bgd,file,current_dir,fn);

            if(a)
            read_file(sb,bgd,file,a);
            else
            {
                cout << "File Not Found!!" << endl;
                cout << endl;
            }
        }

        else
        {
            cout << "Command Not Found" << endl;
            cout << endl;
        }
    }

    return 0;
}
