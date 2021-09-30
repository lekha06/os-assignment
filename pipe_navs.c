#include<pipe.h>
#include<context.h>
#include<memory.h>
#include<lib.h>
#include<entry.h>
#include<file.h>


// Per process info for the pipe.
struct pipe_info_per_process {

    // TODO:: Add members as per your need...
    u32 pid;
     int is_readopen;
    int is_writeopen;
};

// Global information for the pipe.
struct pipe_info_global {

    char *pipe_buff;    // Pipe buffer: DO NOT MODIFY THIS.

    // TODO:: Add members as per your need...
    int read_pos;
    int write_pos;
     int buffer_offset;
};

// Pipe information structure.
// NOTE: DO NOT MODIFY THIS STRUCTURE.
struct pipe_info {

    struct pipe_info_per_process pipe_per_proc [MAX_PIPE_PROC];
    struct pipe_info_global pipe_global;

};


// Function to allocate space for the pipe and initialize its members.
struct pipe_info* alloc_pipe_info () {
	
    // Allocate space for pipe structure and pipe buffer.
    struct pipe_info pipe = (struct pipe_info)os_page_alloc(
    );
    char* buffer = (char*) os_page_alloc(OS_DS_REG);//What is OS_DS_REG

    // Assign pipe buffer.
    pipe->pipe_global.pipe_buff = buffer;//pipe_info_global wala buffweer

    /**
     *  TODO:: Initializing pipe fields
     *  
     *  Initialize per process fields for this pipe.
     *  Initialize global fields for this pipe.
     *
     */

    // Return the pipe.
    pipe->pipe_global.read_pos=0;
    pipe->pipe_global.write_pos=0;
    pipe->pipe_global.buffer_offset=0;
    pipe->pipe_per_proc[0].pid=get_current_ctx()->pid;
    pipe->pipe_per_proc[0].is_readopen=1;
    pipe->pipe_per_proc[0].is_writeopen=1;

    for(int i=1;i<MAX_PIPE_PROC;i++){
        pipe->pipe_per_proc[i].pid=-1;
        pipe->pipe_per_proc[i].is_readopen=1;
        pipe->pipe_per_proc[i].is_writeopen=1;
    }
  
    return pipe;

}

// Function to free pipe buffer and pipe info object.
// NOTE: DO NOT MODIFY THIS FUNCTION.
void free_pipe (struct file *filep) {

    os_page_free(OS_DS_REG, filep->pipe->pipe_global.pipe_buff);
    os_page_free(OS_DS_REG, filep->pipe);

}

// Fork handler for the pipe.
int do_pipe_fork (struct exec_context *child, struct file *filep) {

    /**
     *  TODO:: Implementation for fork handler
     *
     *  You may need to update some per process or global info for the pipe.
     *  This handler will be called twice since pipe has 2 file objects.
     *  Also consider the limit on no of processes a pipe can have.
     *  Return 0 on success.
     *  Incase of any error return -EOTHERS.
     *
     */



    //You can use get_current_ctx() function to get context (PCB) structure
    // using this context you can get the pid

    // Return successfully.
  
int parent_pid = get_current_ctx()->pid;
    int i0;
    for(i0 = 0; i0 < MAX_PIPE_PROC; i0++){
        if(filep->pipe->pipe_per_proc[i0].pid == parent_pid){
            break;
        }
    }
    int flags = filep->mode;
     for(int i = 0; i < MAX_PIPE_PROC; i++){
        if(filep->pipe->pipe_per_proc[i].pid == child->pid){
            return 0;
        }
     }
    for(int i = 0; i < MAX_PIPE_PROC; i++){
        if(filep->pipe->pipe_per_proc[i].pid == -1){
            filep->pipe->pipe_per_proc[i].pid = child->pid;
            if((flags & O_READ) == O_READ)
                filep->pipe->pipe_per_proc[i].is_readopen = filep->pipe->pipe_per_proc[i0].is_readopen;
            if((flags & O_WRITE) == O_WRITE)
                filep->pipe->pipe_per_proc[i].is_writeopen = filep->pipe->pipe_per_proc[i0].is_writeopen;
            return 0;
        }
    }
    return -EOTHERS;
    // / ref_count is used to count no of processes refering to that file object.
}

// Function to close the pipe ends and free the pipe when necessary.
long pipe_close (struct file *filep) {

    /**
     *  TODO:: Implementation of Pipe Close
     *
     *  Close the read or write end of the pipe depending upon the file
     *      object's mode.
     *  You may need to update some per process or global info for the pipe.
     *  Use free_pipe() function to free pipe buffer and pipe object,
     *      whenever applicable.
     *  After successful close, it return 0.
     *  Incase of any error return -EOTHERS.
     *
     */

    // int ret_value;

    // if(filep->type==PIPE){
    //      // last read port of the pipe
    //   if(filep->mode == O_READ) {

    //     // Check  write port of the pipe is already closed
    //     if(filep->pipe->pipe_global.is_writeopen==0) {
    //       free_pipe(filep->pipe);
    //     }
    //     else {//close it
    //       filep->pipe->pipe_global.is_readopen = 0;
    //     }
        
    //   }

    //   // If this is the last write port of the pipe
    //   if(filep->mode == O_WRITE) {
    //     if(filep->pipe->pipe_global.is_readopen==0) {
  
    //       free_pipe(filep->pipe);
    //     }
    //     else {
    //       filep->pipe->pipe_global.is_writeopen = 0;
    //     }
    //   }
    // }

    // // Close the file and return.
    // ret_value = file_close (filep);         // DO NOT MODIFY THIS LINE.

    // // And return.
    // return ret_value;

    
    int ret_value;
 
    u32 pid = get_current_ctx()->pid;
 
    int flags = filep->mode;
 
    if((flags & O_READ) == O_READ){
        for(int i = 0; i < MAX_PIPE_PROC; i++){
            if(filep->pipe->pipe_per_proc[i].pid == pid){
                filep->pipe->pipe_per_proc[i].is_readopen = 0;
                break;
            }
        }
    }
    if((flags & O_WRITE) == O_WRITE){
        for(int i = 0; i < MAX_PIPE_PROC; i++){
            if(filep->pipe->pipe_per_proc[i].pid == pid){
                filep->pipe->pipe_per_proc[i].is_writeopen = 0;
                break;
            }
        }
    }
 
    int i0, count = 0;
    for(int i = 0; i < MAX_PIPE_PROC; i++){
        if(count > 1)
                break;
        if(filep->pipe->pipe_per_proc[i].pid != -1){
            i0 = i;
            count++;
        }
    }
 
    if(count == 1){
        if(filep->pipe->pipe_per_proc[i0].is_readopen == 0 && filep->pipe->pipe_per_proc[i0].is_writeopen == 0)
            free_pipe(filep);
    }
 
 
    // Close the file and return.
    ret_value = file_close (filep);         // DO NOT MODIFY THIS LINE.
 
    // And return.
    return ret_value;
 


}

// Check whether passed buffer is valid memory location for read or write.
int is_valid_mem_range (unsigned long buff, u32 count, int access_bit) {

    /**
     *  TODO:: Implementation for buffer memory range checking
     *
     *  Check whether passed memory range is suitable for read or write.
     *  If access_bit == 1, then it is asking to check read permission.
     *  If access_bit == 2, then it is asking to check write permission.
     *  If range is valid then return 1.
     *  Incase range is not valid or have some permission issue return -EBADMEM.
     *
     */

    int ret_value = -EBADMEM;
    //can we assume that the buffer which the user provides, will lie in only one of the vm_areas/mm_segments and not be a part of multiple of them
    // Return the finding.
    struct exec_context *current = get_ctx_current();

    if ( access_bit && current->vm_area->access_flags != 0){
        if(current->vm_area->vm_start <= buff) {
            if( buff <= current->vm_area->vm_end && buff + count<= current->vm_area->vm_end){
                return 1;
            }
        }
    else if( access_bit && current->mms->access_flags != 0){
            if( current->mms->start <= buff){
                if( buff <= current->mms->end && buff + count <= current->mms->end){
                    return 1;
                }
            }
        }
    }

    // Return the finding.
    return ret_value;

}

// Function to read given no of bytes from the pipe.
int pipe_read (struct file *filep, char *buff, u32 count) {
//buff is where i store
//count is till i have to read...start from its read start if it is oepm
    /**
     *  TODO:: Implementation of Pipe Read
     *
     *  Read the data from pipe buffer and write to the provided buffer.
     *  If count is greater than the present data size in the pipe then just read
     *       that much data.
     *  Validate file object's access right.
     *  On successful read, return no of bytes read.
     *  Incase of Error return valid error code.
     *       -EACCES: In case access is not valid.
     *       -EINVAL: If read end is already closed.
     *       -EOTHERS: For any other errors.
     *
     */
    

    // struct pipe_info *pipept = filep->pipe;

    // if(pipept->pipe_per_proc->is_readopen==0)return -EINVAL;

    // count = (pipept->pipe_global.write_pos - pipept->pipe_global.read_pos)>count?count:(pipept->pipe_global.write_pos - pipept->pipe_global.read_pos);

    // for(int i = 0; i<count; ++i) {
    //     buff[i] = pipept->pipe_global.pipe_buff[(pipept->pipe_global.read_pos+i)%MAX_PIPE_SIZE ];
    // }

    // pipept->pipe_global.read_pos = (pipept->pipe_global.read_pos + count)%MAX_PIPE_SIZE;

    // // Return no of bytes read.
    // return count;




   if(filep->mode != O_READ)return -EACCES;
    if(filep == NULL || filep->type != PIPE) return -EINVAL;
    struct pipe_info *pipept = filep->pipe;
     if(pipept->pipe_per_proc->is_readopen==0)return -EINVAL;

    char *pipe_buff = pipept->pipe_global.pipe_buff;
    int last_read_pos = pipept->pipe_global.read_pos;
    int i=last_read_pos;
    int j=0;
    while(j<count){//Implemented Queue
        if(filep->pipe->pipe_global.buffer_offset==4096){//If queue length is more than 4096
            return -EACCES;
        }
        if(i==4096){
            i=0;
        }
        if(!pipe_buff[i]){
            break;
        }
        buff[j]=pipe_buff[i];
        i++;
        j++;
        filep->pipe->pipe_global.buffer_offset--;
    }
    filep->pipe->pipe_global.read_pos=i;
    return j;

}

// Function to write given no of bytes to the pipe.
int pipe_write (struct file *filep, char *buff, u32 count) {
//buff is the string 
//count is the length of the string 

    /**
     *  TODO:: Implementation of Pipe Write
     *
     *  Write the data from the provided buffer to the pipe buffer.
     *  If count is greater than available space in the pipe then just write data
     *       that fits in that space.
     *  Validate file object's access right.
     *  On successful write, return no of written bytes.
     *  Incase of Error return valid error code.
     *       -EACCES: In case access is not valid.
     *       -EINVAL: If write end is already closed.
     *       -EOTHERS: For any other errors.
     *
     */
    // if( filep->mode != O_WRITE)return -EACCES;
    // if(filep == NULL || filep->type != PIPE ) return -EINVAL;
    //  struct pipe_info *pipept = filep->pipe;

    // long int maxlength = MAX_PIPE_SIZE - ((pipept->pipe_global.write_pos) - (pipept->pipe_global.read_pos));

    // if(maxlength < count) count=maxlength;

    //  for(int i = 0; i < count; ++i) {
    //     pipept->pipe_global.pipe_buff[(pipept->pipe_global.write_pos+i)%MAX_PIPE_SIZE] = buff[i];
    // }
    //  pipept->pipe_global.write_pos = (pipept->pipe_global.write_pos + count)%MAX_PIPE_SIZE;

    // // Return no of bytes written.
    // return count;


      if( filep->mode != O_WRITE)return -EACCES;
    if(filep == NULL || filep->type != PIPE ) return -EINVAL;
     struct pipe_info *pipept = filep->pipe;
    char * pipe_buff = pipept->pipe_global.pipe_buff;
    int last_write_pos = pipept->pipe_global.write_pos;
    int i=last_write_pos;
    int j=0;
    while(j<count){//Implemented Queue
        if(pipept->pipe_global.buffer_offset == 4096){//Error if queue lenght>4096
            return -EACCES;
        }
        if(i==4096){
            i=0;
        }
        if(!buff[j]){
            break;
        }
        pipe_buff[i]=buff[j];
        i++;
        j++;
        pipept->pipe_global.buffer_offset++;
    }
    pipept->pipe_global.write_pos=i;
    return j;

}

// Function to create pipe.
int create_pipe (struct exec_context *current, int *fd) {

    /**
     *  TODO:: Implementation of Pipe Create
     *
     *  Find two free file descriptors.
     *  Create two file objects for both ends by invoking the alloc_file() function. 
     *  Create pipe_info object by invoking the alloc_pipe_info() function and
     *       fill per process and global info fields.
     *  Fill the fields for those file objects like type, fops, etc.
     *  Fill the valid file descriptor in *fd param.
     *  On success, return 0.
     *  Incase of Error return valid Error code.
     *       -ENOMEM: If memory is not enough.
     *       -EOTHERS: Some other errors.
     *
     */

    //  inputs valid or not
    if(current == NULL || fd == NULL || (fd+1) == NULL) return -EINVAL;

    //  Find two free file descriptors.
    int input_fd = 3;
    while(current->files[input_fd] != NULL && input_fd < MAX_OPEN_FILES) input_fd++;
    if(input_fd == MAX_OPEN_FILES) return -EINVAL;

    int output_fd = input_fd+1;
    while(current->files[output_fd] != NULL && output_fd < MAX_OPEN_FILES) output_fd++;
    if(output_fd >= MAX_OPEN_FILES) return -EINVAL;

     //file objects
    struct file * inputfile = alloc_file();
    struct file * outputfile = alloc_file();
    if(!inputfile){
        return -ENOMEM;
    }
    if(!outputfile){
        return -ENOMEM;
    }
     //Create pipe_info object
    struct pipe_info * pipe_ptr = alloc_pipe_info();
    if(!pipe_ptr)return -ENOMEM;

    //need not fill cause already intilaised h

     inputfile->type=PIPE;//Initialize file ptr
    inputfile->mode=O_READ;
    inputfile->pipe=pipe_ptr;
    //ye read write close we have to implement
    inputfile->fops->read=pipe_read;
    inputfile->fops->write=pipe_write;
    inputfile->fops->close=pipe_close;

    current->files[input_fd] = inputfile;

    outputfile->type=PIPE;
    outputfile->mode=O_WRITE;
    outputfile->pipe=pipe_ptr;
    outputfile->fops->read=pipe_read;
    outputfile->fops->write=pipe_write;  
    outputfile->fops->close=pipe_close;
    
    current->files[output_fd] = outputfile;

     // Assign file descriptors
        fd[0] = input_fd;
        fd[1] = output_fd;

    // Simple return.
    return 0;

}
