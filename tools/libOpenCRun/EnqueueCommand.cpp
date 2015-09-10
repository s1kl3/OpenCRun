
#include "CL/opencl.h"

#include "Utils.h"

#include "opencrun/Core/Command.h"
#include "opencrun/Core/CommandQueue.h"
#include "opencrun/Core/Event.h"
#include "opencrun/Core/MemoryObject.h"

#define CL_MAP_FIELD_ALL   \
  (CL_MAP_READ |     \
   CL_MAP_WRITE |     \
   CL_MAP_WRITE_INVALIDATE_REGION)

static inline bool clValidMapField(cl_mem_flags map_flags) {
  return !(map_flags & ~CL_MAP_FIELD_ALL);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue command_queue,
                    cl_mem buffer,
                    cl_bool blocking_read,
                    size_t offset,
                    size_t cb,
                    void *ptr,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  if(!cb)
    return CL_INVALID_VALUE;
    
  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  cl_int ErrCode;

  opencrun::EnqueueReadBufferBuilder Bld(*Queue, buffer, ptr);
  opencrun::Command *Cmd = Bld.SetBlocking(blocking_read)
                              .SetCopyArea(offset, cb)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBufferRect(cl_command_queue command_queue,
                        cl_mem buffer,
                        cl_bool blocking_read,
                        const size_t *buffer_origin,
                        const size_t *host_origin,
                        const size_t *region,
                        size_t buffer_row_pitch,
                        size_t buffer_slice_pitch,
                        size_t host_row_pitch,
                        size_t host_slice_pitch,
                        void *ptr,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event) CL_API_SUFFIX__VERSION_1_1 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;
    
  opencrun::CommandQueue *Queue;
  
  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);
  
  cl_int ErrCode;
  
  opencrun::EnqueueReadBufferRectBuilder Bld(*Queue, buffer, ptr);
  opencrun::Command *Cmd = Bld.SetBlocking(blocking_read)
                              .SetRegion(region)
                              .SetTargetOffset(host_origin, host_row_pitch, host_slice_pitch)
                              .SetSourceOffset(buffer_origin, buffer_row_pitch, buffer_slice_pitch)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);
                              
  if(!Cmd)
    return ErrCode;
  
  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue command_queue,
                     cl_mem buffer,
                     cl_bool blocking_write,
                     size_t offset,
                     size_t cb,
                     const void *ptr,
                     cl_uint num_events_in_wait_list,
                     const cl_event *event_wait_list,
                     cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;
  
  if(!cb)
    return CL_INVALID_VALUE;
    
  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  cl_int ErrCode;

  opencrun::EnqueueWriteBufferBuilder Bld(*Queue, buffer, ptr);
  opencrun::Command *Cmd = Bld.SetBlocking(blocking_write)
                              .SetCopyArea(offset, cb)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBufferRect(cl_command_queue command_queue,
                         cl_mem buffer,
                         cl_bool blocking_write,
                         const size_t *buffer_origin,
                         const size_t *host_origin,
                         const size_t *region,
                         size_t buffer_row_pitch,
                         size_t buffer_slice_pitch,
                         size_t host_row_pitch,
                         size_t host_slice_pitch,
                         const void *ptr,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_event *event) CL_API_SUFFIX__VERSION_1_1 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;
    
  opencrun::CommandQueue *Queue;
  
  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);
  
  cl_int ErrCode;
  
  opencrun::EnqueueWriteBufferRectBuilder Bld(*Queue, buffer, ptr);
  opencrun::Command *Cmd = Bld.SetBlocking(blocking_write)
                              .SetRegion(region)
                              .SetTargetOffset(buffer_origin, buffer_row_pitch, buffer_slice_pitch)
                              .SetSourceOffset(host_origin, host_row_pitch, host_slice_pitch)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);
                              
  if(!Cmd)
    return ErrCode;
  
  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillBuffer(cl_command_queue command_queue,
                    cl_mem buffer,
                    const void *pattern,
                    size_t pattern_size,
                    size_t offset,
                    size_t size,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event) CL_API_SUFFIX__VERSION_1_2 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;
    
  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  cl_int ErrCode;

  opencrun::EnqueueFillBufferBuilder Bld(*Queue, buffer, pattern);
  opencrun::Command *Cmd = Bld.SetPatternSize(pattern_size)
                              .SetFillRegion(offset, size)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);  
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBuffer(cl_command_queue command_queue,
                    cl_mem src_buffer,
                    cl_mem dst_buffer,
                    size_t src_offset,
                    size_t dst_offset,
                    size_t cb,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  if(!cb)
    return CL_INVALID_VALUE;
  
  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  cl_int ErrCode;
  
  opencrun::EnqueueCopyBufferBuilder Bld(*Queue, dst_buffer, src_buffer);
  opencrun::Command *Cmd = Bld.SetCopyArea(dst_offset, src_offset, cb)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);  
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferRect(cl_command_queue command_queue,
                        cl_mem src_buffer,
                        cl_mem dst_buffer,
                        const size_t *src_origin,
                        const size_t *dst_origin,
                        const size_t *region,
                        size_t src_row_pitch,
                        size_t src_slice_pitch,
                        size_t dst_row_pitch,
                        size_t dst_slice_pitch,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event) CL_API_SUFFIX__VERSION_1_1 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;
  
  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  cl_int ErrCode;
  
  opencrun::EnqueueCopyBufferRectBuilder Bld(*Queue, dst_buffer, src_buffer);
  opencrun::Command *Cmd = Bld.SetRegion(region)
                              .SetTargetOffset(dst_origin, dst_row_pitch, dst_slice_pitch)
                              .SetSourceOffset(src_origin, src_row_pitch, src_slice_pitch)
                              .CheckCopyOverlap()
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);                          
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadImage(cl_command_queue command_queue,
                   cl_mem image,
                   cl_bool blocking_read,
                   const size_t *origin,
                   const size_t *region,
                   size_t row_pitch,
                   size_t slice_pitch,
                   void *ptr,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  if(!Queue->GetDevice().HasImageSupport())
    return CL_INVALID_OPERATION;

  cl_int ErrCode;

  opencrun::EnqueueReadImageBuilder Bld(*Queue, image, ptr);
  opencrun::Command *Cmd = Bld.SetBlocking(blocking_read)
                              .SetCopyArea(origin, 
                                           region, 
                                           row_pitch, 
                                           slice_pitch)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteImage(cl_command_queue command_queue,
                    cl_mem image,
                    cl_bool blocking_write,
                    const size_t *origin,
                    const size_t *region,
                    size_t input_row_pitch,
                    size_t input_slice_pitch,
                    const void *ptr,
                    cl_uint num_events_in_wait_list,
                    const cl_event *event_wait_list,
                    cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  if(!Queue->GetDevice().HasImageSupport())
    return CL_INVALID_OPERATION;

  cl_int ErrCode;

  opencrun::EnqueueWriteImageBuilder Bld(*Queue, image, ptr);
  opencrun::Command *Cmd = Bld.SetBlocking(blocking_write)
                              .SetCopyArea(origin,
                                           region, 
                                           input_row_pitch, 
                                           input_slice_pitch)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueFillImage(cl_command_queue command_queue,
                   cl_mem image,
                   const void *fill_color,
                   const size_t *origin,
                   const size_t *region,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event) CL_API_SUFFIX__VERSION_1_2 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  if(!Queue->GetDevice().HasImageSupport())
    return CL_INVALID_OPERATION;

  cl_int ErrCode;

  opencrun::EnqueueFillImageBuilder Bld(*Queue, image, fill_color);
  opencrun::Command *Cmd = Bld.SetFillRegion(origin, region)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);  
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImage(cl_command_queue command_queue,
                   cl_mem src_image,
                   cl_mem dst_image,
                   const size_t *src_origin,
                   const size_t *dst_origin,
                   const size_t *region,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  if(!Queue->GetDevice().HasImageSupport())
    return CL_INVALID_OPERATION;

  cl_int ErrCode;

  opencrun::EnqueueCopyImageBuilder Bld(*Queue, dst_image, src_image);
  opencrun::Command *Cmd = Bld.SetCopyArea(dst_origin, src_origin, region)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyImageToBuffer(cl_command_queue command_queue,
                           cl_mem src_image,
                           cl_mem dst_buffer,
                           const size_t *src_origin,
                           const size_t *region,
                           size_t dst_offset,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  if(!Queue->GetDevice().HasImageSupport())
    return CL_INVALID_OPERATION;

  cl_int ErrCode;

  opencrun::EnqueueCopyImageToBufferBuilder Bld(*Queue, dst_buffer, src_image);
  opencrun::Command *Cmd = Bld.SetCopyArea(dst_offset, src_origin, region)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueCopyBufferToImage(cl_command_queue command_queue,
                           cl_mem src_buffer,
                           cl_mem dst_image,
                           size_t src_offset,
                           const size_t *dst_origin,
                           const size_t *region,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  if(!Queue->GetDevice().HasImageSupport())
    return CL_INVALID_OPERATION;

  cl_int ErrCode;

  opencrun::EnqueueCopyBufferToImageBuilder Bld(*Queue, dst_image, src_buffer);
  opencrun::Command *Cmd = Bld.SetCopyArea(dst_origin, region, src_offset)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapBuffer(cl_command_queue command_queue,
                   cl_mem buffer,
                   cl_bool blocking_map,
                   cl_map_flags map_flags,
                   size_t offset,
                   size_t cb,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event,
                   cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_COMMAND_QUEUE);
  
  if(!clValidMapField(map_flags))
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_VALUE);
    
  if(!cb)
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_VALUE);
  
  opencrun::CommandQueue *Queue;
  
  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);
  
  void *MapBuf;

  opencrun::EnqueueMapBufferBuilder Bld(*Queue, buffer);
  opencrun::Command *Cmd = Bld.SetBlocking(blocking_map)
                              .SetMapFlags(map_flags)
                              .SetMapArea(offset, cb)
                              .SetMapBuffer(&MapBuf)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(errcode_ret);

  if(!Cmd) {
    return NULL;
  }

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, errcode_ret);

  if(!Ev) {
    return NULL;
  }
  
  if(event) {
    *event = Ev.get();
    Ev.resetWithoutRelease();
  }
  
  return MapBuf;
}

CL_API_ENTRY void * CL_API_CALL
clEnqueueMapImage(cl_command_queue command_queue,
                  cl_mem image,
                  cl_bool blocking_map,
                  cl_map_flags map_flags,
                  const size_t *origin,
                  const size_t *region,
                  size_t *image_row_pitch,
                  size_t *image_slice_pitch,
                  cl_uint num_events_in_wait_list,
                  const cl_event *event_wait_list,
                  cl_event *event,
                  cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_COMMAND_QUEUE);

  if(!clValidMapField(map_flags))
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_VALUE);

  opencrun::CommandQueue *Queue; 
  
  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  if(!Queue->GetDevice().HasImageSupport())
    RETURN_WITH_ERROR(errcode_ret, CL_INVALID_OPERATION);

  void *MapBuf;

  opencrun::EnqueueMapImageBuilder Bld(*Queue, image);
  opencrun::Command *Cmd = Bld.SetBlocking(blocking_map)
                              .SetMapFlags(map_flags)
                              .SetMapPitches(image_row_pitch, 
                                             image_slice_pitch)
                              .SetMapArea(origin, region)
                              .SetMapBuffer(&MapBuf)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(errcode_ret);

  if(!Cmd) {
    return NULL;
  }

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, errcode_ret);

  if(!Ev) {
    return NULL;
  }
  
  if(event) {
    *event = Ev.get();
    Ev.resetWithoutRelease();
  }
  
  return MapBuf;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueUnmapMemObject(cl_command_queue command_queue,
                        cl_mem memobj,
                        void *mapped_ptr,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;
      
  if(!mapped_ptr)
    return CL_INVALID_VALUE;
  
  opencrun::CommandQueue *Queue;
  opencrun::MemoryObject *MemObj;
  
  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);
  MemObj = llvm::cast<opencrun::MemoryObject>(memobj);
  
  cl_int ErrCode;
  
  if(!MemObj->isValidMapping(mapped_ptr))
    return CL_INVALID_VALUE;
  
  opencrun::EnqueueUnmapMemObjectBuilder Bld(Queue->GetContext(), memobj, mapped_ptr);
  opencrun::Command *Cmd = Bld.SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMigrateMemObjects(cl_command_queue command_queue,
                           cl_uint num_mem_objects,
                           const cl_mem *mem_objects,
                           cl_mem_migration_flags flags,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           cl_event *event) CL_API_SUFFIX__VERSION_1_2 {
  llvm_unreachable("Not yet implemented");
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue command_queue,
                       cl_kernel kernel,
                       cl_uint work_dim,
                       const size_t *global_work_offset,
                       const size_t *global_work_size,
                       const size_t *local_work_size,
                       cl_uint num_events_in_wait_list,
                       const cl_event *event_wait_list,
                       cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;
  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  cl_int ErrCode;

  opencrun::EnqueueNDRangeKernelBuilder Bld(Queue->GetContext(),
                                            Queue->GetDevice(),
                                            kernel,
                                            work_dim,
                                            global_work_size);
  opencrun::Command *Cmd = Bld.SetGlobalWorkOffset(global_work_offset)
                              .SetLocalWorkSize(local_work_size)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueTask(cl_command_queue command_queue,
              cl_kernel kernel,
              cl_uint num_events_in_wait_list,
              const cl_event *event_wait_list,
              cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  cl_uint work_dim = 1;
  const size_t *global_work_offset = NULL;
  const size_t global_work_size[] = { 1 };
  const size_t local_work_size[] = { 1 };

  return clEnqueueNDRangeKernel(command_queue,
                                kernel,
                                work_dim,
                                global_work_offset,
                                global_work_size,
                                local_work_size,
                                num_events_in_wait_list,
                                event_wait_list,
                                event
                                );
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNativeKernel(cl_command_queue command_queue,
                      void (*user_func)(void *),
                      void *args,
                      size_t cb_args,
                      cl_uint num_mem_objects,
                      const cl_mem *mem_list,
                      const void **args_mem_loc,
                      cl_uint num_events_in_wait_list,
                      const cl_event *event_wait_list,
                      cl_event *event) CL_API_SUFFIX__VERSION_1_0 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;
  
  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  cl_int ErrCode;

  opencrun::EnqueueNativeKernel::Arguments RawArgs(args, cb_args);
  opencrun::EnqueueNativeKernelBuilder Bld(Queue->GetContext(),
                                           user_func,
                                           RawArgs);
  opencrun::Command *Cmd = Bld.SetMemoryLocations(num_mem_objects,
                                                  mem_list,
                                                  args_mem_loc)
                              .SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueMarkerWithWaitList(cl_command_queue command_queue,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list,
                            cl_event *event) CL_API_SUFFIX__VERSION_1_2 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  cl_int ErrCode;

  opencrun::EnqueueMarkerBuilder Bld(*Queue);
  opencrun::Command *Cmd = Bld.SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
clEnqueueMarker(cl_command_queue command_queue,
                cl_event *event) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED {
  if(!event)
    return CL_INVALID_VALUE;

  return clEnqueueMarkerWithWaitList(command_queue, 0, NULL, event);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
clEnqueueWaitForEvents(cl_command_queue command_queue,
                        cl_uint num_events,
                        const cl_event *event_list) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED {
  if(!num_events || !event_list)
    return CL_INVALID_VALUE;

  for(unsigned I = 0; I < num_events; ++I)
    if(!event_list[I]) return CL_INVALID_EVENT;

  return clEnqueueBarrierWithWaitList(command_queue, num_events, event_list, NULL);
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueBarrierWithWaitList(cl_command_queue command_queue,
                             cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list,
                             cl_event *event) CL_API_SUFFIX__VERSION_1_2 {
  if(!command_queue)
    return CL_INVALID_COMMAND_QUEUE;

  opencrun::CommandQueue *Queue;

  Queue = llvm::cast<opencrun::CommandQueue>(command_queue);

  cl_int ErrCode;

  opencrun::EnqueueBarrierBuilder Bld(*Queue);
  opencrun::Command *Cmd = Bld.SetWaitList(num_events_in_wait_list,
                                           event_wait_list)
                              .Create(&ErrCode);

  if(!Cmd)
    return ErrCode;

  llvm::IntrusiveRefCntPtr<opencrun::Event> Ev = Queue->Enqueue(*Cmd, &ErrCode);

  if(!Ev)
    return ErrCode;

  RETURN_WITH_EVENT(event, Ev);
}

CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
clEnqueueBarrier(cl_command_queue command_queue) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED {
  return clEnqueueBarrierWithWaitList(command_queue, 0, NULL, NULL);
}
