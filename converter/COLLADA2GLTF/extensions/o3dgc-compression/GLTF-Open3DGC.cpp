// Copyright (c) 2013, Fabrice Robinet.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "GLTF.h"
#include "../GLTF-OpenCOLLADA.h"
#include "../GLTFConverterContext.h"
#include "GLTF-Open3DGC.h"

#include "o3dgcSC3DMCEncodeParams.h"
#include "o3dgcIndexedFaceSet.h"
#include "o3dgcSC3DMCEncoder.h"
#include "o3dgcSC3DMCDecoder.h"

#include "o3dgcTimer.h"
#include "o3dgcDVEncodeParams.h"
#include "o3dgcDynamicVectorEncoder.h"
#include "o3dgcDynamicVectorDecoder.h"

#define DUMP_O3DGC_OUTPUT 0

using namespace o3dgc;
namespace GLTF
{
    
    void SaveIFSUnsignedShortArray(std::ofstream & fout, const std::string & name, unsigned int a, const unsigned short * const tab, unsigned long nElement, unsigned long dim)
    {
        if (tab)
        {
            fout << name << "\t" << a << "\t" << nElement << "\t" << dim << std::endl;
        }
        else
        {
            fout << name << "\t" << a << "\t" << 0 << "\t" << 0 << std::endl;
            return;
        }
        if (!tab) return;
        for (unsigned long i = 0; i < nElement; ++i){
            fout << "[" << i << "]\t";
            for (unsigned long j = 0; j < dim; ++j){
                fout << tab[i*dim + j] << "\t";
            }
            fout << std::endl;
        }
    }
    
    void SaveIFSIntArray(std::ofstream & fout, const std::string & name, unsigned int a, const long * const tab, unsigned long nElement, unsigned long dim)
    {
        if (tab)
        {
            fout << name << "\t" << a << "\t" << nElement << "\t" << dim << std::endl;
        }
        else
        {
            fout << name << "\t" << a << "\t" << 0 << "\t" << 0 << std::endl;
            return;
        }
        if (!tab) return;
        for (unsigned long i = 0; i < nElement; ++i){
            fout << "[" << i << "]\t";
            for (unsigned long j = 0; j < dim; ++j){
                fout << tab[i*dim + j] << "\t";
            }
            fout << std::endl;
        }
    }
    
    
    void SaveIFSFloatArray(std::ofstream & fout, const std::string & name, unsigned int a, const Real * const tab, unsigned long nElement, unsigned long dim)
    {
        if (tab)
        {
            fout << name << "\t" << a << "\t" << nElement << "\t" << dim << std::endl;
        }
        else
        {
            fout << name << "\t" << a << "\t" << 0 << "\t" << 0 << std::endl;
            return;
        }
        for (unsigned long i = 0; i < nElement; ++i){
            fout << "[" << i << "]\t";
            for (unsigned long j = 0; j < dim; ++j){
                fout << tab[i*dim + j] << "\t";
            }
            fout << std::endl;
        }
    }
    bool SaveIFS(std::string & fileName, const IndexedFaceSet<unsigned short> & ifs)
    {
        std::ofstream fout;
        fout.open(fileName.c_str());
        if (!fout.fail())
        {
            SaveIFSUnsignedShortArray(fout, "* CoordIndex", 0, (const unsigned short * const) ifs.GetCoordIndex(), ifs.GetNCoordIndex(), 3);
            SaveIFSIntArray(fout, "* MatID", 0, (const long * const) ifs.GetIndexBufferID(), ifs.GetNCoordIndex(), 1);
            SaveIFSFloatArray(fout, "* Coord", 0, ifs.GetCoord(), ifs.GetNCoord(), 3);
            SaveIFSFloatArray(fout, "* Normal", 0, ifs.GetNormal(), ifs.GetNNormal(), 3);
            for(unsigned long a = 0; a < ifs.GetNumFloatAttributes(); ++a)
            {
                SaveIFSFloatArray(fout, "* FloatAttribute", a, ifs.GetFloatAttribute(a), ifs.GetNFloatAttribute(a), ifs.GetFloatAttributeDim(a));
            }
            for(unsigned long a = 0; a < ifs.GetNumIntAttributes(); ++a)
            {
                SaveIFSIntArray(fout, "* IntAttribute", a, ifs.GetIntAttribute(a), ifs.GetNIntAttribute(a), ifs.GetIntAttributeDim(a));
            }
            fout.close();
        }
        else
        {
            std::cout << "Not able to create file" << std::endl;
            return false;
        }
        return true;
    }
    
    void testDecode(shared_ptr <GLTFMesh> mesh, BinaryStream &bstream)
    {
        SC3DMCDecoder <unsigned short> decoder;
        IndexedFaceSet <unsigned short> ifs;
        unsigned char* outputData;
        
        decoder.DecodeHeader(ifs, bstream);
        
        unsigned int vertexSize = ifs.GetNCoord() * 3 * sizeof(float);
        unsigned int normalSize = ifs.GetNNormal() * 3 * sizeof(float);
        unsigned int texcoordSize = ifs.GetNFloatAttribute(0) * 2 * sizeof(float);
        unsigned int indicesSize = ifs.GetNCoordIndex() * 3 * sizeof(unsigned short);
        
        outputData = (unsigned char*)malloc(vertexSize + normalSize + texcoordSize + indicesSize);
        
        size_t vertexOffset = indicesSize;
        
        float* uncompressedVertices = (Real * const )(outputData + vertexOffset);
        
        ifs.SetCoordIndex((unsigned short * const ) outputData );
        ifs.SetCoord((Real * const )uncompressedVertices);
        
        if (ifs.GetNNormal() > 0) {
            ifs.SetNormal((Real * const )(outputData + indicesSize + vertexSize));
        }
        if (ifs.GetNFloatAttribute(0)) {
            ifs.SetFloatAttribute(0, (Real * const )(outputData + indicesSize + vertexSize + normalSize));
        }
        
        decoder.DecodePlayload(ifs, bstream);
        
        //---
        
        shared_ptr <GLTFMeshAttribute> meshAttribute = mesh->getMeshAttribute(POSITION, 0);
        
        meshAttribute->computeMinMax();
        const double* min = meshAttribute->getMin();
        const double* max = meshAttribute->getMax();
        
        float* vertices = (float*)meshAttribute->getBufferView()->getBufferDataByApplyingOffset();
        
        printf("coord nb:%d\n",(int)meshAttribute->getCount());
        printf("min: %f %f %f\n", min[0], min[1], min[2]);
        printf("max: %f %f %f\n", max[0], max[1], max[2]);
        
        float maxQuantError[3];
        maxQuantError[0] = (max[0] - min[0]) / (float)(2^12 - 1);
        maxQuantError[1] = (max[1] - min[1]) / (float)(2^12 - 1);
        maxQuantError[2] = (max[2] - min[2]) / (float)(2^12 - 1);
        
        if (meshAttribute->getCount() == ifs.GetNCoord()) {
            for (size_t i = 0 ; i < (meshAttribute->getCount() * 3) ; i++ ) {
                float error = vertices[i] - uncompressedVertices[i];
                
                if (error > maxQuantError[i%3]) {
                    printf("%d:input:%f compressed:%f\n",(int) i%3, vertices[i], uncompressedVertices[i]);
                    printf("delta is: %f\n", error);
                } else {
                    //printf("ok\n");
                }
            }
            
        } else {
            printf("Fatal error: vertex count do not match\n");
        }
        
        free(outputData);
    }
    
    //All these limitations will be fixed in coming iterations:
    //Do we have only triangles, only one set of texcoord and no skinning ?
    //TODO:Also check that the same buffer is not shared by 2 different semantics or set
    bool canEncodeOpen3DGCMesh(shared_ptr <GLTFMesh> mesh)
    {
        PrimitiveVector primitives = mesh->getPrimitives();
        unsigned int primitivesCount =  (unsigned int)primitives.size();
        
        for (unsigned int i = 0 ; i < primitivesCount ; i++) {
            shared_ptr<GLTF::GLTFPrimitive> primitive = primitives[i];
            if (primitive->getType() != "TRIANGLES") {
                return false;
            }
        }
        
        return true;
    }
    
    void encodeOpen3DGCMesh(shared_ptr <GLTFMesh> mesh,
                            shared_ptr<JSONObject> floatAttributeIndexMapping,
                            const GLTFConverterContext& converterContext)
    {
        o3dgc::SC3DMCEncodeParams params;
        o3dgc::IndexedFaceSet <unsigned short> ifs;

        //setup options
        int qcoord    = 12;
        int qtexCoord = 10;
        int qnormal   = 10;
        int qcolor   = 10;
        int qWeights = 8;
        
        GLTFOutputStream *outputStream = converterContext._compressionOutputStream;
        size_t bufferOffset = outputStream->length();
        
        O3DGCSC3DMCPredictionMode floatAttributePrediction = O3DGC_SC3DMC_PARALLELOGRAM_PREDICTION;
        
        unsigned int nFloatAttributes = 0;
        
        PrimitiveVector primitives = mesh->getPrimitives();
        unsigned int primitivesCount =  (unsigned int)primitives.size();
        unsigned int allIndicesCount = 0;
        unsigned int allTrianglesCount = 0;
        
        std::vector <unsigned int> trianglesPerPrimitive;
        
        //First run through primitives to gather the number of indices and infer the number of triangles.
        for (unsigned int i = 0 ; i < primitivesCount ; i++) {
            shared_ptr<GLTF::GLTFPrimitive> primitive = primitives[i];
            shared_ptr <GLTF::GLTFIndices> uniqueIndices = primitive->getUniqueIndices();
            unsigned int indicesCount = (unsigned int)(uniqueIndices->getCount());
            //FIXME: assumes triangles, but we are guarded from issues by canEncodeOpen3DGCMesh
            allIndicesCount += indicesCount;
            trianglesPerPrimitive.push_back(indicesCount / 3);
        }
        
        //Then we setup the matIDs array and at the same time concatenate all triangle indices
        unsigned long *primitiveIDs = (unsigned long*)malloc(sizeof(unsigned long) * (allIndicesCount / 3));
        unsigned long *primitiveIDsPtr = primitiveIDs;
        unsigned short* allConcatenatedIndices = (unsigned short*)malloc(allIndicesCount * sizeof(unsigned short));
        unsigned short* allConcatenatedIndicesPtr = allConcatenatedIndices;
        
        for (unsigned int i = 0 ; i < trianglesPerPrimitive.size() ; i++) {
            unsigned int trianglesCount = trianglesPerPrimitive[i];
            for (unsigned int j = 0 ; j < trianglesCount ; j++) {
                primitiveIDsPtr[j] = i;
            }
            primitiveIDsPtr += trianglesCount;
            allTrianglesCount += trianglesCount;
            shared_ptr<GLTF::GLTFPrimitive> primitive = primitives[i];
            shared_ptr <GLTF::GLTFIndices> uniqueIndices = primitive->getUniqueIndices();
            unsigned int indicesCount = (unsigned int)(uniqueIndices->getCount());
            unsigned int* indicesPtr = (unsigned int*)uniqueIndices->getBufferView()->getBufferDataByApplyingOffset();
            for (unsigned int j = 0 ; j < indicesCount ; j++) {
                allConcatenatedIndicesPtr[j] = indicesPtr[j];
            }
            allConcatenatedIndicesPtr += indicesCount;
        }
        
        //FIXME:Open3DGC SetNCoordIndex is not a good name here (file against o3dgc)
        ifs.SetNCoordIndex(allTrianglesCount);
        ifs.SetCoordIndex((unsigned short * const ) allConcatenatedIndices);
        ifs.SetIndexBufferID(primitiveIDs);
        
        size_t vertexCount = 0;
        
        std::vector <GLTF::Semantic> semantics = mesh->allSemantics();
        for (unsigned int i = 0 ; i < semantics.size() ; i ++) {
            GLTF::Semantic semantic  = semantics[i];
            
            size_t attributesCount = mesh->getMeshAttributesCountForSemantic(semantic);
            
            for (size_t j = 0 ; j < attributesCount ; j++) {
                shared_ptr <GLTFMeshAttribute> meshAttribute = mesh->getMeshAttribute(semantic, j);
                vertexCount = meshAttribute->getCount();
                size_t componentsPerAttribute = meshAttribute->getComponentsPerAttribute();
                char *buffer = (char*)meshAttribute->getBufferView()->getBufferDataByApplyingOffset();
                switch (semantic) {
                    case POSITION:
                        params.SetCoordQuantBits(qcoord);
                        params.SetCoordPredMode(floatAttributePrediction);
                        ifs.SetNCoord(vertexCount);
                        ifs.SetCoord((Real * const)buffer);
                        break;
                    case NORMAL:
                        params.SetNormalQuantBits(qnormal);
                        params.SetNormalPredMode(O3DGC_SC3DMC_SURF_NORMALS_PREDICTION);
                        ifs.SetNNormal(vertexCount);
                        ifs.SetNormal((Real * const)buffer);
                        break;
                    case TEXCOORD:
                        params.SetFloatAttributeQuantBits(nFloatAttributes, qtexCoord);
                        params.SetFloatAttributePredMode(nFloatAttributes, floatAttributePrediction);
                        ifs.SetNFloatAttribute(nFloatAttributes, vertexCount);
                        ifs.SetFloatAttributeDim(nFloatAttributes, componentsPerAttribute);
                        ifs.SetFloatAttributeType(nFloatAttributes, O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_TEXCOORD);
                        ifs.SetFloatAttribute(nFloatAttributes, (Real * const)buffer);
                        floatAttributeIndexMapping->setUnsignedInt32(meshAttribute->getID(), nFloatAttributes);
                        nFloatAttributes++;
                        break;
                    case COLOR:
                        params.SetFloatAttributeQuantBits(nFloatAttributes, qcolor);
                        params.SetFloatAttributePredMode(nFloatAttributes, floatAttributePrediction);
                        ifs.SetNFloatAttribute(nFloatAttributes, vertexCount);
                        ifs.SetFloatAttributeDim(nFloatAttributes, componentsPerAttribute);
                        ifs.SetFloatAttributeType(nFloatAttributes, O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_COLOR);
                        ifs.SetFloatAttribute(nFloatAttributes, (Real * const)buffer);
                        floatAttributeIndexMapping->setUnsignedInt32(meshAttribute->getID(), nFloatAttributes);
                        nFloatAttributes++;
                        break;
                    case WEIGHT:
                        params.SetFloatAttributeQuantBits(nFloatAttributes, qWeights);
                        params.SetFloatAttributePredMode(nFloatAttributes, O3DGC_SC3DMC_DIFFERENTIAL_PREDICTION);
                        ifs.SetNFloatAttribute(nFloatAttributes, vertexCount);
                        ifs.SetFloatAttributeDim(nFloatAttributes, componentsPerAttribute);
                        ifs.SetFloatAttributeType(nFloatAttributes, O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_WEIGHT);
                        ifs.SetFloatAttribute(nFloatAttributes, (Real * const)buffer);
                        floatAttributeIndexMapping->setUnsignedInt32(meshAttribute->getID(), nFloatAttributes);
                        nFloatAttributes++;
                        break;
                    case JOINT:
                        /*
                         params.SetIntAttributePredMode(nIntAttributes, O3DGC_SC3DMC_DIFFERENTIAL_PREDICTION);
                         ifs.SetNIntAttribute(nIntAttributes, jointIDs.size() / numJointsPerVertex);
                         ifs.SetIntAttributeDim(nIntAttributes, numJointsPerVertex);
                         ifs.SetIntAttributeType(nIntAttributes, O3DGC_IFS_INT_ATTRIBUTE_TYPE_JOINT_ID);
                         ifs.SetIntAttribute(nIntAttributes, (long * const ) & (jointIDs[0]));
                         nIntAttributes++;
                         */
                        params.SetFloatAttributeQuantBits(nFloatAttributes, 10);
                        params.SetFloatAttributePredMode(nFloatAttributes, O3DGC_SC3DMC_PARALLELOGRAM_PREDICTION);
                        ifs.SetNFloatAttribute(nFloatAttributes, vertexCount);
                        ifs.SetFloatAttributeDim(nFloatAttributes, componentsPerAttribute);
                        ifs.SetFloatAttributeType(nFloatAttributes, O3DGC_IFS_FLOAT_ATTRIBUTE_TYPE_UNKOWN);
                        ifs.SetFloatAttribute(nFloatAttributes, (Real * const)buffer);
                        floatAttributeIndexMapping->setUnsignedInt32(meshAttribute->getID(), nFloatAttributes);
                        nFloatAttributes++;
                        break;
                    default:
                        break;
                }
            }
        }
        
        params.SetNumFloatAttributes(nFloatAttributes);
        ifs.SetNumFloatAttributes(nFloatAttributes);
        shared_ptr<JSONObject> compressionObject = static_pointer_cast<JSONObject>(mesh->getExtensions()->createObjectIfNeeded("Open3DGC-compression"));
        
        ifs.ComputeMinMax(O3DGC_SC3DMC_MAX_ALL_DIMS);
        BinaryStream bstream(vertexCount * 8);
        SC3DMCEncoder <unsigned short> encoder;
        shared_ptr<JSONObject> compressedData(new JSONObject());
        compressedData->setInt32("verticesCount", vertexCount);
        compressedData->setInt32("indicesCount", allIndicesCount);
        //Open3DGC binary is disabled
        params.SetStreamType(converterContext.compressionMode == "binary" ? O3DGC_STREAM_TYPE_BINARY : O3DGC_STREAM_TYPE_ASCII);
#if DUMP_O3DGC_OUTPUT
        static int dumpedId = 0;
        COLLADABU::URI outputURI(converterContext.outputFilePath.c_str());
        std::string outputFilePath = outputURI.getPathDir() + GLTFUtils::toString(dumpedId) + ".txt";
        dumpedId++;
        SaveIFS(outputFilePath, ifs);
#endif
        encoder.Encode(params, ifs, bstream);
        
        compressedData->setString("mode", converterContext.compressionMode);
        compressedData->setUnsignedInt32("count", bstream.GetSize());
        compressedData->setUnsignedInt32("type", converterContext.profile->getGLenumForString("UNSIGNED_BYTE"));
        compressedData->setUnsignedInt32("byteOffset", bufferOffset);
        compressedData->setValue("floatAttributesIndexes", floatAttributeIndexMapping);
        
        compressionObject->setValue("compressedData", compressedData);
        
        //testDecode(mesh, bstream);
        outputStream->write((const char*)bstream.GetBuffer(0), bstream.GetSize());
        
        if (ifs.GetCoordIndex()) {
            free(ifs.GetCoordIndex());
        }
        
        if (primitiveIDs) {
            free(primitiveIDs);
        }
    }
    
    void encodeDynamicVector(float *buffer, size_t componentsCount, size_t count, const GLTFConverterContext& converterContext) {
        GLTFOutputStream *outputStream = converterContext._compressionOutputStream;
        Real max[componentsCount];
        Real min[componentsCount];
        
        O3DGCStreamType streamType = converterContext.compressionMode == "ascii" ? O3DGC_STREAM_TYPE_ASCII : O3DGC_STREAM_TYPE_BINARY;
        
        DynamicVector dynamicVector;
        dynamicVector.SetVectors(buffer);
        dynamicVector.SetDimVector(componentsCount);
        dynamicVector.SetMax(max);
        dynamicVector.SetMin(min);
        dynamicVector.SetNVector(count);
        dynamicVector.SetStride(componentsCount);
        dynamicVector.ComputeMinMax(O3DGC_SC3DMC_MAX_SEP_DIM);//O3DGC_SC3DMC_MAX_ALL_DIMS        
        DVEncodeParams params;
        
        params.SetQuantBits(componentsCount == 1 ? 11 : 17); //HACK, if that's 1 component it is the TIME and 10 bits is OK
        params.SetStreamType(streamType);
        
        DynamicVectorEncoder encoder;
        encoder.SetStreamType(streamType);
        Timer timer;
        timer.Tic();
        BinaryStream bstream(componentsCount * count * 16);
        encoder.Encode(params, dynamicVector, bstream);
        timer.Toc();
        outputStream->write((const char*)bstream.GetBuffer(), bstream.GetSize());
        
        /*
        if (componentsCount == 4) {
            DynamicVector  dynamicVector1;
            DynamicVectorDecoder decoder;
            decoder.DecodeHeader(dynamicVector1, bstream);
            dynamicVector1.SetStride(dynamicVector1.GetDimVector());
            
            std::vector<Real> oDV;
            std::vector<Real> oDVMin;
            std::vector<Real> oDVMax;
            oDV.resize(dynamicVector1.GetNVector() * dynamicVector1.GetDimVector());
            oDVMin.resize(dynamicVector1.GetDimVector());
            oDVMax.resize(dynamicVector1.GetDimVector());
            dynamicVector1.SetVectors(& oDV[0]);
            dynamicVector1.SetMin(& oDVMin[0]);
            dynamicVector1.SetMax(& oDVMax[0]);
            decoder.DecodePlayload(dynamicVector1, bstream);
            
            float* dbuffer = (float*)dynamicVector1.GetVectors();
            
            printf("****dump axis-angle. Axis[3] / Angle[1]\n");
            for (int i = 0 ; i < count ; i++) {
                int offset = i * 4;
                printf("[raw]%f %f %f %f [10bits]%f %f %f %f\n",
                       buffer[offset+0],buffer[offset+1],buffer[offset+2],buffer[offset+3],
                       dbuffer[offset+0],dbuffer[offset+1],dbuffer[offset+2],dbuffer[offset+3]);
            }
        }
        */
    }
    
    /*
     Handles Parameter creation / addition
     */
    static std::string __SetupSamplerForParameter(shared_ptr <GLTFAnimation> cvtAnimation,
                                                  GLTFAnimation::Parameter *parameter) {
        shared_ptr<JSONObject> sampler(new JSONObject());
        std::string name = parameter->getID();
        std::string samplerID = cvtAnimation->getSamplerIDForName(name);
        sampler->setString("input", "TIME");           //FIXME:harcoded for now
        sampler->setString("interpolation", "LINEAR"); //FIXME:harcoded for now
        sampler->setString("output", name);
        cvtAnimation->samplers()->setValue(samplerID, sampler);
        
        return samplerID;
    }
    
    static shared_ptr <GLTFAnimation::Parameter> __SetupAnimationParameter(shared_ptr <GLTFAnimation> cvtAnimation,
                                                                           const std::string& parameterSID,
                                                                           const std::string& parameterType) {
        //setup
        shared_ptr <GLTFAnimation::Parameter> parameter(new GLTFAnimation::Parameter(parameterSID));
        parameter->setCount(cvtAnimation->getCount());
        parameter->setType(parameterType);
        __SetupSamplerForParameter(cvtAnimation, parameter.get());
        
        cvtAnimation->parameters()->push_back(parameter);
        
        return parameter;
    }    
    
    /*
     Handles Parameter creation / addition / write
     */
    void setupAndWriteAnimationParameter(shared_ptr <GLTFAnimation> cvtAnimation,
                                         const std::string& parameterSID,
                                         const std::string& parameterType,
                                         unsigned char* buffer, size_t length,
                                         GLTFConverterContext &converterContext) {
        bool shouldEncodeOpen3DGC = converterContext.compressionType == "Open3DGC";

        GLTFOutputStream *outputStream = shouldEncodeOpen3DGC ? converterContext._compressionOutputStream : converterContext._animationOutputStream;
        //setup
        shared_ptr <GLTFAnimation::Parameter> parameter = __SetupAnimationParameter(cvtAnimation, parameterSID, parameterType);
        shared_ptr <GLTFProfile> profile = converterContext.profile;
        //write
        size_t byteOffset = outputStream->length();
        parameter->setByteOffset(byteOffset);
        
        if (shouldEncodeOpen3DGC) {
            unsigned int glType = profile->getGLenumForString(parameterType);
            size_t componentsCount = profile->getComponentsCountForType(glType);
            if (componentsCount) {
                encodeDynamicVector((float*)buffer, componentsCount, cvtAnimation->getCount(), converterContext);
                
                size_t bytesCount = outputStream->length() - byteOffset;
                
                shared_ptr<JSONObject> compressionObject = parameter->extensions()->createObjectIfNeeded("Open3DGC-compression");
                shared_ptr<JSONObject> compressionDataObject = compressionObject->createObjectIfNeeded("compressedData");
                
                compressionDataObject->setUnsignedInt32("byteOffset", byteOffset);
                compressionDataObject->setUnsignedInt32("count", bytesCount);
                compressionDataObject->setString("mode", converterContext.compressionMode);
                compressionDataObject->setUnsignedInt32("type", profile->getGLenumForString("UNSIGNED_BYTE"));
            }
        } else {
            outputStream->write((const char*)buffer, length);
        }
    }
    
    
}
