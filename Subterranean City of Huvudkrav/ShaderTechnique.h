#pragma once
#include "Resource.h"
#include <string>
struct VSstruct
{
	ID3D11VertexShader* vertexShader;
	string name;
};
struct HSstruct
{
	ID3D11HullShader* hullShader;
	string name;
};
struct DSstruct
{
	ID3D11DomainShader* domainShader;
	string name;
};
struct GSstruct
{
	ID3D11GeometryShader* geometryShader;
	string name;
};
struct PSstruct
{
	ID3D11PixelShader* pixelShader;
	string name;
};
class ShaderTechnique
{
private:
	static vector<VSstruct> *vertexShaders;
	static vector<HSstruct> *hullShaders;
	static vector<DSstruct> *domainShaders;
	static vector<GSstruct> *geometryShaders;
	static vector<PSstruct> *pixelShaders;
	int insertVertexShader(string shaderDir, string shaderName);
	int insertHullShader(string shaderDir, string shaderName);
	int insertDomainShader(string shaderDir, string shaderName);
	int insertGeometryShader(string shaderDir, string shaderName);
	int insertPixelShader(string shaderDir, string shaderName);
	int vertexShaderIndex;
	int hullShaderIndex;
	int domainShaderIndex;
	int geometryShaderIndex;
	int pixelShaderIndex;

	ID3DBlob* vertexBlob;
public:
	ShaderTechnique(string vsPath, string vs, string gsPath,string gs, string psPath, string ps);
	ShaderTechnique(string vsPath, string vs, string hsPath,string hs,string dsPath,string ds, string psPath, string ps);
	~ShaderTechnique();

	static void cleanUp();
	void useTechnique();

	LPVOID getInputSignature()		{ return vertexBlob->GetBufferPointer(); }
	SIZE_T getInputSignatureSize()	{ return vertexBlob->GetBufferSize(); }
};

