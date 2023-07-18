#include "_reg_common_gpu.h"
#include "_reg_tools.h"

#include "_reg_nmi_gpu.h"
#include "_reg_ssd_gpu.h"
#include "_reg_localTransformation.h"
#include "_reg_localTransformation_gpu.h"
#include "_reg_resampling.h"
#include "_reg_resampling_gpu.h"

#define EPS 0.001
#define SIZE 128
#define BIN 68

int main(int argc, char **argv)
{
   if(argc!=3)
   {
      fprintf(stderr, "Usage: %s <dim> <type>\n", argv[0]);
      fprintf(stderr, "<dim>\tImages dimension (2,3)\n");
      fprintf(stderr, "<type>\tTest type:\n");
      fprintf(stderr, "\t\t- spatial gradient (\"spa\")\n");
      fprintf(stderr, "\t\t- nmi gradient (\"nmig\")\n");
      fprintf(stderr, "\t\t- ssd values (\"ssd\")\n");
      fprintf(stderr, "\t\t- ssd gradient (\"ssdg\")\n");
      return EXIT_FAILURE;
   }
   int dimension=atoi(argv[1]);
   char *type=argv[2];

   // Check and setup the GPU card
   CUcontext ctx;
   if(cudaCommon_setCUDACard(&ctx, true))
      return EXIT_FAILURE;

   // Create the input images
   int nii_dim[8]= {dimension,SIZE,SIZE,SIZE,1,1,1,1};
   if(dimension==2)
      nii_dim[3]=1;
   nifti_image *reference = nifti_make_new_nim(nii_dim, NIFTI_TYPE_FLOAT32, true);
   nifti_image *floating = nifti_make_new_nim(nii_dim, NIFTI_TYPE_FLOAT32, true);
   reg_checkAndCorrectDimension(reference);
   reg_checkAndCorrectDimension(floating);
   // Fill the input images with random value
   float *refPtr=static_cast<float *>(reference->data);
   float *floPtr=static_cast<float *>(floating->data);
   for(size_t i=0; i<reference->nvox; ++i)
   {
      refPtr[i] = (float)rand()/(float)RAND_MAX * (float)(BIN-5) + 2.f;
      floPtr[i] = (float)rand()/(float)RAND_MAX * (float)(BIN-5) + 2.f;
   }
   // Define a mask array
   int *mask = (int *)malloc(reference->nvox*sizeof(int));
   for(size_t i=0; i<reference->nvox; ++i)
      mask[i]=i;

   // Create a identity deformation field
   nii_dim[0]=5;
   nii_dim[5]=dimension;
   nifti_image *field = nifti_make_new_nim(nii_dim, NIFTI_TYPE_FLOAT32, true);
   reg_checkAndCorrectDimension(field);
   memset(field->data, 0, field->nvox*field->nbyper);
   reg_getDeformationFromDisplacement(field);

   // Create the spatial gradient image
   nifti_image *spaGradient = nifti_make_new_nim(nii_dim, NIFTI_TYPE_FLOAT32, true);
   reg_checkAndCorrectDimension(spaGradient);

   // Compute the spatial gradient on the host
   reg_getImageGradient(floating,
                        spaGradient,
                        field,
                        mask,
                        1,  // interpolation type | 1=linear
                        2); // padding value

   // Allocate the reference and floating image on the GPU
   cudaArray *reference_gpu=NULL;
   cudaCommon_allocateArrayToDevice<float>(&reference_gpu, reference->dim);
   cudaCommon_transferNiftiToArrayOnDevice<float>(&reference_gpu, reference);
   cudaArray *floating_cuda_array_gpu=NULL;
   cudaCommon_allocateArrayToDevice<float>(&floating_cuda_array_gpu, floating->dim);
   cudaCommon_transferNiftiToArrayOnDevice<float>(&floating_cuda_array_gpu, floating);
   float *floating_gpu=NULL;
   cudaCommon_allocateArrayToDevice<float>(&floating_gpu, floating->dim);
   cudaCommon_transferNiftiToArrayOnDevice<float>(&floating_gpu, floating);
   // Create a mask array on the device
   int *mask_gpu = NULL;
   NR_CUDA_SAFE_CALL(cudaMalloc(&mask_gpu, reference->nvox*sizeof(int)));
   NR_CUDA_SAFE_CALL(cudaMemcpy(mask_gpu, mask, reference->nvox*sizeof(int),
                                cudaMemcpyHostToDevice));
   // Create a spatial gradient array on the GPU
   float4 *spaGradient_gpu=NULL;
   cudaCommon_allocateArrayToDevice<float4>(&spaGradient_gpu, spaGradient->dim);

   float maxDifferenceSPA=0;
   float maxDifferenceNMIG=0;
   float maxDifferenceSSD=0;
   float maxDifferenceSSDG=0;

   if(strcmp(type,"spa")==0)
   {
      // Allocate the deformation field on the device
      float4 *field_gpu=NULL;
      cudaCommon_allocateArrayToDevice<float4>(&field_gpu, field->dim);
      cudaCommon_transferNiftiToArrayOnDevice<float4>(&field_gpu, field);
      // compute the spatial gradient on the device
      reg_getImageGradient_gpu(floating,
                               &floating_cuda_array_gpu,
                               &field_gpu,
                               &spaGradient_gpu,
                               reference->nvox,
                               2); // padding value

      nifti_image *spaGradient2 = nifti_make_new_nim(nii_dim, NIFTI_TYPE_FLOAT32, true);
      reg_checkAndCorrectDimension(spaGradient2);
      cudaCommon_transferFromDeviceToNifti<float4>(spaGradient2, &spaGradient_gpu);
      maxDifferenceSPA=reg_test_compare_images(spaGradient,spaGradient2);
      nifti_image_free(spaGradient2);
      cudaCommon_free(&field_gpu);
   }
   else
   {
      // The spatial gradient is transfer on the device
      cudaCommon_transferNiftiToArrayOnDevice<float4>(&spaGradient_gpu, spaGradient);
   }

   if(strcmp(type,"nmig")==0)
   {
      bool activeTimePoint=true;
      unsigned short binNumber = BIN;
      unsigned short totalBinNumber = BIN*(BIN+2);
      // Allocate the required joint histogram
      double *proJHisto = (double *)malloc(totalBinNumber*sizeof(double));
      double *logJHisto = (double *)malloc(totalBinNumber*sizeof(double));
      double *entropies = (double *)malloc(4*sizeof(double));
      // Compute the NMI
      reg_getNMIValue<float>(reference,
                             floating,
                             &activeTimePoint,
                             &binNumber,
                             &binNumber,
                             &totalBinNumber,
                             &proJHisto,
                             &logJHisto,
                             &entropies,
                             mask);
      // Allocate two images to store the computed NMI gradient
      nifti_image *nmiGradient1 = nifti_make_new_nim(nii_dim, NIFTI_TYPE_FLOAT32, true);
      reg_checkAndCorrectDimension(nmiGradient1);
      nifti_image *nmiGradient2 = nifti_make_new_nim(nii_dim, NIFTI_TYPE_FLOAT32, true);
      reg_checkAndCorrectDimension(nmiGradient2);
      // Allocate an array on the GPU to store the log prob joint histogram
      float *logJHisto_gpu=NULL;
      NR_CUDA_SAFE_CALL(cudaMalloc(&logJHisto_gpu,
                                   BIN*(BIN+2)*sizeof(float)))
      float *logJHisto_temp=NULL;
      NR_CUDA_SAFE_CALL(cudaMallocHost(&logJHisto_temp, BIN*(BIN+2)*sizeof(float)))
      for(unsigned int i=0; i<BIN*(BIN+2); i++)
         logJHisto_temp[i]=static_cast<float>(logJHisto[i]);
      NR_CUDA_SAFE_CALL(cudaMemcpy(logJHisto_gpu,
                                   logJHisto_temp,
                                   BIN*(BIN+2)*sizeof(float),
                                   cudaMemcpyHostToDevice))
      NR_CUDA_SAFE_CALL(cudaFreeHost(logJHisto_temp))
      // Allocate an array on the GPU to compute the NMI gradient on the device
      float4 *nmiGradient_gpu=NULL;
      cudaCommon_allocateArrayToDevice<float4>(&nmiGradient_gpu, nmiGradient2->dim);
      // Compute the NMI gradient on the host
      reg_getVoxelBasedNMIGradient3D<float>(reference,
                                            floating,
                                            &activeTimePoint,
                                            &binNumber,
                                            &binNumber,
                                            &logJHisto,
                                            &entropies,
                                            spaGradient,
                                            nmiGradient1,
                                            mask);
      // Compute the NMI gradient on the device
      reg_getVoxelBasedNMIGradient_gpu(reference,
                                       &reference_gpu,
                                       &floating_gpu,
                                       &spaGradient_gpu,
                                       &logJHisto_gpu,
                                       &nmiGradient_gpu,
                                       &mask_gpu,
                                       reference->nvox,
                                       entropies,
                                       BIN,
                                       BIN);
      // Transfer the result from the device to the hosts
      cudaCommon_transferFromDeviceToNifti<float4>(nmiGradient2, &nmiGradient_gpu);
      //Compare the result
      reg_tools_multiplyValueToImage(nmiGradient1,nmiGradient1,reference->nvox); // xnvox
      reg_tools_multiplyValueToImage(nmiGradient2,nmiGradient2,reference->nvox); // xnvox
      maxDifferenceNMIG=reg_test_compare_images(nmiGradient1,nmiGradient2);
      // Free arrays
      free(proJHisto);
      free(logJHisto);
      cudaCommon_free(&logJHisto_gpu);
      cudaCommon_free(&nmiGradient_gpu);
      nifti_image_free(nmiGradient1);
      nifti_image_free(nmiGradient2);
   }
   else if(strcmp(type,"ssd")==0)
   {
      bool activeTimePoint=true;
      float *values=(float *)malloc(reference->nt*sizeof(float));
      float ssd_cpu = (float)reg_getSSDValue<float>(reference,
                      floating,
                      &activeTimePoint,
                      NULL,
                      mask,
                      values);
      float ssd_gpu = reg_getSSDValue_gpu(reference,
                                          &reference_gpu,
                                          &floating_gpu,
                                          &mask_gpu,
                                          (int)reference->nvox);
      printf("ssd cpu %g\n", ssd_cpu);
      printf("ssd gpu %g\n", ssd_gpu);
      maxDifferenceSSD = fabsf(ssd_cpu-ssd_gpu);
   }
   else if(strcmp(type,"ssdg")==0)
   {
      // Allocate two images to store the computed NMI gradient
      nifti_image *ssdGradient1 = nifti_make_new_nim(nii_dim, NIFTI_TYPE_FLOAT32, true);
      reg_checkAndCorrectDimension(ssdGradient1);
      nifti_image *ssdGradient2 = nifti_make_new_nim(nii_dim, NIFTI_TYPE_FLOAT32, true);
      reg_checkAndCorrectDimension(ssdGradient2);
      // Compute the ssd gradient on the host
      bool activeTimePoint=true;
      float values[1]= {1.f};
      reg_getVoxelBasedSSDGradient<float>(reference,
                                          floating,
                                          &activeTimePoint,
                                          spaGradient,
                                          ssdGradient1,
                                          NULL,
                                          mask,
                                          values
                                         );
      // Allocate an array on the GPU to compute the NMI gradient on the device
      float4 *ssdGradient_gpu=NULL;
      cudaCommon_allocateArrayToDevice<float4>(&ssdGradient_gpu, ssdGradient2->dim);
      // Compute the ssd gradient on the host
      reg_getVoxelBasedSSDGradient_gpu(reference,
                                       &reference_gpu,
                                       &floating_gpu,
                                       &spaGradient_gpu,
                                       &ssdGradient_gpu,
                                       68.f,
                                       &mask_gpu,
                                       reference->nvox
                                      );
      // Transfer the result from the device to the hosts
      cudaCommon_transferFromDeviceToNifti<float4>(ssdGradient2, &ssdGradient_gpu);
      //Compare the result
      reg_tools_multiplyValueToImage(ssdGradient1,ssdGradient1,reference->nvox); // * nvox
      reg_tools_multiplyValueToImage(ssdGradient2,ssdGradient2,reference->nvox); // * nvox
      maxDifferenceSSDG=reg_test_compare_images(ssdGradient1,ssdGradient2);
      // Free allocate images and array
      cudaCommon_free(&ssdGradient_gpu);
      nifti_image_free(ssdGradient1);
      nifti_image_free(ssdGradient2);
   }

   if(maxDifferenceSPA>EPS)
   {
      fprintf(stderr,
              "[dim=%i] Spatial gradient difference too high: %g\n",
              dimension,
              maxDifferenceSPA);
      return EXIT_FAILURE;
   }
   else if(maxDifferenceNMIG>EPS)
   {
      fprintf(stderr,
              "[dim=%i] NMI gradient difference too high: %g\n",
              dimension,
              maxDifferenceNMIG);
      return EXIT_FAILURE;
   }
   else if(maxDifferenceSSD>EPS)
   {
      fprintf(stderr,
              "[dim=%i] SSD difference too high: %g\n",
              dimension,
              maxDifferenceSSD);
      return EXIT_FAILURE;
   }
   else if(maxDifferenceSSDG>EPS)
   {
      fprintf(stderr,
              "[dim=%i] SSD gradient difference too high: %g\n",
              dimension,
              maxDifferenceSSDG);
      return EXIT_FAILURE;
   }

   // Clean the allocate arrays and images
   nifti_image_free(reference);
   nifti_image_free(floating);
   nifti_image_free(field);
   free(mask);
   cudaCommon_free(&reference_gpu);
   cudaCommon_free(&floating_cuda_array_gpu);
   cudaCommon_free(&floating_gpu);
   cudaCommon_free(&mask_gpu);

   cudaCommon_unsetCUDACard(&ctx);

   return EXIT_SUCCESS;
}

