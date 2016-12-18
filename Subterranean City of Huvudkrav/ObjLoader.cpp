#include "ObjLoader.h"

ObjLoader *ObjLoader::instance = NULL;
ObjLoader::ObjLoader()
{
	vList			= new vector<XMFLOAT3>();
	nList			= new vector<XMFLOAT3>();
	tList			= new vector<XMFLOAT2>();
	missingNormals	= new vector<VertexFaceLink>();
	countedNormals	= new vector<VertexFaceLink>();
	tanList			= new vector<XMFLOAT3>();
	bitanList		= new vector<XMFLOAT3>();
}

bool ObjLoader::LoadMesh(vector<SimpleMeshVertex> *outVertices, vector<IndexGroup> *outIGroups,
						vector<Material> *outMaterials, const char *filename, vector<string> *texturePaths)
{
	string temp = filename;
	path.clear();

	int i;
	for(i = temp.size()-1; i >= 0; i--)
	{
		if(temp.at(i) == '/')
			break;
	}
	for(int k = 0; k <= i; k++)
		path.push_back(temp.at(k));

	vSize			= 0;
	file			= new ifstream(filename);
	
	
	if( file->is_open() )
	{
		if(vList->size() > 0)
			vList->clear();
		if(nList->size() > 0)
			nList->clear();
		if(tList->size() > 0)
			tList->clear();
		if(missingNormals->size() > 0)
			missingNormals->clear();
		if(countedNormals->size() > 0)
			countedNormals->clear();

		while( !file->eof() )
		{
			*file >> controlSymbol;

			switch(controlSymbol)
			{
				case 'v': //vertex data
					readVertex();
					break;
				case 'f': //face data
					readFace(outVertices);
					break;
				case 'm': //material
					readMaterial(outMaterials, texturePaths);
					break;
				case 'o':

				case 'g':
					readGroup(outIGroups);
					break;
				case 'u':
					readUse(outMaterials, outIGroups);
					break;
				case 's':
					readSmooth();
					break;
				default:
				{
					char buffer[512];
					file->getline( buffer, 512 );
					break;
				}
			}
		}
		if((outIGroups)->size() > 0)
			(outIGroups)->at((outIGroups)->size()-1).iEnd = vSize-1;
	}
	else
	{
		cout << "Unable to open mesh file: " << filename << endl;
		return false;
	}

	if(missingNormals->size() > 0)
		generateNormals(outVertices);

	SAFE_DELETE( file );
	return true;
}

bool ObjLoader::findFile(const char *path)
{
	ifstream file(path);
    if(file.fail()){
        return false;
    }
	return true;
}

void ObjLoader::readVertex()
{
	controlSymbol = file->peek();

	switch ( controlSymbol )
	{
		case 't': // It's a texcoord
		{
			*file >> controlSymbol >> x >> y;

			// Add the texcoord to the vector.
			tList->push_back( XMFLOAT2(x,1-y) );
			break;
		}

		case 'n': // It's a normal
		{
			*file >> controlSymbol >> x >> y >> z;

			// Add the normal to the vector
			nList->push_back( XMFLOAT3(x,y,z) );
			break;
		}

		case 'p': // Parameter space vertex (not currently supported)
		{
			cout << "Error: Tried to load parameter space vertex. Ignored";
			break;
		}

		default: // It's a vertexposition
		{
			*file >> x >> y >> z;

			// Add the position to the vector.
			vList->push_back( XMFLOAT3(x,y,z) );
			break;
		}
	}
}

void ObjLoader::readFace(vector<SimpleMeshVertex> *vertices)
{
	for( int i = 0; i < 3; i++ )
	{
		// Vertex info.
		*file >> vals[0];
		// Feed the info to the final vertex list.
		vertices->push_back(SimpleMeshVertex());
		vertices->at(vSize).pos			= vList->at( vals[0]-1 );

		separator = file->peek();
		if(separator != '/')
		{
			//add default empty tex coord to be fixed later (or not)
			XMFLOAT2 tempTex = XMFLOAT2(0,0);
			tList->push_back(tempTex);
			vertices->at(vSize).texC	= tempTex;
			//add default empty normals to be fixed later
			VertexFaceLink temp;
			temp.normalIndex = vals[0]-1;
			temp.vertexIndex = vSize;
			missingNormals->push_back(temp);
			vSize++;
			continue;
		}
		*file >> separator;
		*file >> vals[1];
		vertices->at(vSize).texC		= tList->at( vals[1]-1 );

		separator = file->peek();
		if(separator != '/')
		{
			//add default empty normals to be fixed later
			VertexFaceLink temp;
			temp.normalIndex = vals[0]-1;
			temp.vertexIndex = vSize;
			missingNormals->push_back(temp);
			vSize++;
			continue;
		}
		*file >> separator;
		*file >> vals[2];
		vertices->at(vSize).normal		= nList->at( vals[2]-1 );

		//add this link so we have all connections other than the ones without normals
		VertexFaceLink temp;
		temp.normalIndex = vals[0]-1;
		temp.vertexIndex = vSize;
		countedNormals->push_back(temp);

		vSize++;
	}
}

void ObjLoader::readMaterial(vector<Material> *materials, vector<string> *texturePaths)
{
	string temp;

	*file >> temp;

	char buffer[64];

	if( temp == "tllib" )
	{
		*file >> temp;

		readMtlFile(temp, materials, texturePaths);
	}
	else
		file->getline( buffer, 64 );
}

void ObjLoader::readMtlFile(string filename, vector<Material> *materials, vector<string> *texturePaths)
{
	// Opening the material-file 
	mtlFile = new ifstream( path + filename.c_str() );
	if( mtlFile->is_open() )
	{
		char controlsymbol;
		char buffer[128];
		string s;
		while( !mtlFile->eof() )
		{
			*mtlFile >> controlsymbol;
			switch( controlsymbol )
			{
				case 'K': // Material's light coeff.
				{
					*mtlFile >> controlsymbol;
					switch( controlsymbol )
					{
						case 'a': // Ambient coeff.
							*mtlFile >> materials->at(materials->size()-1).ka.x >> materials->at(materials->size()-1).ka.y >> materials->at(materials->size()-1).ka.z;
							break;
						case 'd': // Diffuse coeff.
							*mtlFile >> materials->at(materials->size()-1).kd.x >> materials->at(materials->size()-1).kd.y >> materials->at(materials->size()-1).kd.z;
							break;
						case 's': // Specular coeff.
							*mtlFile >> materials->at(materials->size()-1).ks.x >> materials->at(materials->size()-1).ks.y >> materials->at(materials->size()-1).ks.z;
							break;
						case 'e':
							*mtlFile >> materials->at(materials->size()-1).ke.x >> materials->at(materials->size()-1).ke.y >> materials->at(materials->size()-1).ke.z;
							break;
						default:
							//if it's not a valid coeff. the line will be thrown away
							mtlFile->getline( buffer, 128 );
					}
					break;
				}
				case 'T':
				{
					*mtlFile >> controlsymbol;
					switch(controlsymbol)
					{
						case 'f':
							*mtlFile >> materials->at(materials->size()-1).tf.x >> materials->at(materials->size()-1).tf.y >> materials->at(materials->size()-1).tf.z;
							break;
						case 'r': // Transparancy
							*mtlFile >>materials->at(materials->size()-1).tr;
							break;
						default:
							mtlFile->getline( buffer, 128 );
					}
					break;
				}
				case 'N':
				{
					*mtlFile >> controlsymbol;
					switch(controlsymbol)
					{
						case 'i':
							*mtlFile >> materials->at(materials->size()-1).ni;
							break;
						case 's':
							*mtlFile >> materials->at(materials->size()-1).ns;
							break;
						default:
							mtlFile->getline( buffer, 128 );
					}
					break;
				}
				case 'm': // Maps.
				{
					*mtlFile >> s;
					if( s == "ap_Ka" )
						addMap(&materials->at(materials->size()-1).map_ka, texturePaths);
					else if( s == "ap_Kd" )
						addMap(&materials->at(materials->size()-1).map_kd, texturePaths);
					else if( s == "ap_Ks" )
						addMap(&materials->at(materials->size()-1).map_ks, texturePaths);
					else if( s == "ap_Ke" )
						addMap(&materials->at(materials->size()-1).map_ke, texturePaths);
					else if( s == "ap_Ns" )
						addMap(&materials->at(materials->size()-1).map_ns, texturePaths);
					else if( s == "ap_Tr" )
						addMap(&materials->at(materials->size()-1).map_tr, texturePaths);
					else if( s == "ap_Bump" || s == "ap_bump" )
						addMap(&materials->at(materials->size()-1).map_bump, texturePaths);
					else if( s == "ap_Disp" || s == "ap_disp")
						addMap(&materials->at(materials->size()-1).map_disp, texturePaths);
					else if( s == "ap_Decal" || s == "ap_decal" )
						addMap(&materials->at(materials->size()-1).map_decal, texturePaths);
					else if( s == "ap_Normal" || s == "ap_normal")
						addMap(&materials->at(materials->size()-1).map_normal, texturePaths);
					else
						mtlFile->getline( buffer, 128 );
					break;
				}
				case 'i':
				{
					*mtlFile >> s;
					if( s == "llum" )
						*mtlFile >> materials->at(materials->size()-1).illum;
					else
						mtlFile->getline(buffer, 128);
					break;
				}
				case 'n':
				{
					s.clear();
					*mtlFile >> s;
					if( s == "ewmtl" ) // Add new material to the vector.
					{
						s.clear();
						*mtlFile >> s;
						materials->push_back(Material());
						materials->at(materials->size()-1).name = s;
					}
					break;
				}
				default:
				{
					// throw away line
					mtlFile->getline( buffer, 128 );
					break;
				}
			}
			s = "";
		}
	}
	else
		cout << "Unable to open Material file: " << filename << endl;

	//deleting the filereader
	SAFE_DELETE( mtlFile );
}

//adds index to a map if the file can be located
void ObjLoader::addMap(int *map, vector<string> *texturePaths)
{
	string filePath;
	*mtlFile >> filePath;	//add the path/filename.fileformat
	if( !findFile( filePath.c_str() ) )
	{
		cout << "could not find or open " << filePath << endl;
		filePath = path + filePath;		//try adding model's path before filename
		if( !findFile( filePath.c_str() ) )
		{
			cout << "could not find or open " << filePath << endl;
			return;
		}
		cout << "tried " << filePath << " and was successful." << endl;
	}
	//connect map_ka with the new texturePath

	for(int k = texturePaths->size()-1; k >= 0; k--)
		{
			if(texturePaths->at(k) == filePath)
			{
				*map = k;
				return;
			}
		}

	*map = texturePaths->size();
	texturePaths->push_back(filePath);
}

void ObjLoader::readGroup(vector<IndexGroup> *outIGroups)
{
	bool cont = true;
	string name;
	*file >> name;
	unsigned int i;
	//check for group name in case it already exists
	for(i = 0; i < outIGroups->size() && cont; i++)
	{
		if( name.find( outIGroups->at( i ).name ) != string::npos )
		{
			cont = false;
		}
	}
	//no group found. Create new group.
	if( i == outIGroups->size())
	{
		outIGroups->push_back( IndexGroup() );
		outIGroups->at(outIGroups->size()-1).iStart = vSize;
		outIGroups->at(outIGroups->size()-1).iEnd = vSize;
		outIGroups->at(i).name = name;
		//set the end index of the previous group
		if(i > 0 && vSize > 0)
			outIGroups->at(i-1).iEnd = vSize-1;
	}
	//current index is now of the group.
	curGroupIndex = i;
}

void ObjLoader::readUse(vector<Material> *materials, vector<IndexGroup> *outIGroups)
{
	string t;

	*file >> t;
	char buffer[32];

	// if the complete word is "usemtl"
	if( t == "semtl" )
	{
		// Get the material name to use.
		*file >> t;
		// Get the material with the name from the mtl reader
		// and add it to the current group.
		//groups->at( groupIndex )->material = mtlreader->getMaterial( t );

		for(unsigned int i = 0; i < materials->size(); i++)
		{
			if(materials->at(i).name  == t)
				outIGroups->at(curGroupIndex).material = &materials->at(i);
		}
	}
	else
		file->getline( buffer, 32 );
}
void ObjLoader::readSmooth()
{
	char buffer[32];
	//ignore smoothing
	file->getline( buffer, 32 );
	cout << "ObjLoader: Ignoring smoothing group." << endl;
}

void ObjLoader::generateNormals(vector<SimpleMeshVertex> *vertices)
{
	XMVECTOR normal, vecn1, vecn2, temp;
	int tempInt;
	//generate new sharp normals (fast way)
	if(missingNormals->size() == vertices->size())
		for(unsigned int i = 0; i < vertices->size(); i+=3)
		{
			vecn1 = XMLoadFloat3( &vertices->at(i+1).pos) - XMLoadFloat3(&vertices->at(i).pos );
			vecn2 = XMLoadFloat3( &vertices->at(i+2).pos) - XMLoadFloat3(&vertices->at(i).pos );
			normal = XMVector3Normalize( XMVector3Cross(vecn1, vecn2) );
			XMStoreFloat3(&vertices->at(i).normal, normal);
			XMStoreFloat3(&vertices->at(i+1).normal, normal);
			XMStoreFloat3(&vertices->at(i+2).normal, normal);
		}
	else //slow way
	{
		for(unsigned int i = 0; i < missingNormals->size(); i++)
		{
			tempInt = missingNormals->at(i).normalIndex % 3;
			if(tempInt == 0)
			{
				vecn1 = XMLoadFloat3( &vertices->at(i+1).pos) - XMLoadFloat3(&vertices->at(i).pos );
				vecn2 = XMLoadFloat3( &vertices->at(i+2).pos) - XMLoadFloat3(&vertices->at(i).pos );
			}
			else if(tempInt == 1)
			{
				vecn1 = XMLoadFloat3( &vertices->at(i).pos) - XMLoadFloat3(&vertices->at(i-1).pos );
				vecn2 = XMLoadFloat3( &vertices->at(i+1).pos) - XMLoadFloat3(&vertices->at(i-1).pos );
			}
			else
			{
				vecn1 = XMLoadFloat3( &vertices->at(i-1).pos) - XMLoadFloat3(&vertices->at(i-2).pos );
				vecn2 = XMLoadFloat3( &vertices->at(i).pos) - XMLoadFloat3(&vertices->at(i-2).pos );
			}
			normal = XMVector3Normalize( XMVector3Cross(vecn1, vecn2) );
			XMStoreFloat3(&vertices->at(i).normal, normal);
		}
	}
	while(nList->size() < vList->size())
		nList->push_back( XMFLOAT3(0,0,0) );

	//add normals together
	for(unsigned int i = 0; i < missingNormals->size(); i++)
	{
		nList->at(missingNormals->at(i).normalIndex).x += vertices->at(missingNormals->at(i).vertexIndex).normal.x;
		nList->at(missingNormals->at(i).normalIndex).y += vertices->at(missingNormals->at(i).vertexIndex).normal.y;
		nList->at(missingNormals->at(i).normalIndex).z += vertices->at(missingNormals->at(i).vertexIndex).normal.z;
	}
	//normalize and add back to vertices
	for(unsigned int i = 0; i < missingNormals->size(); i++)
	{
		temp = XMVector3Normalize( XMLoadFloat3( &nList->at( missingNormals->at(i).normalIndex ) ) );
		XMStoreFloat3(&vertices->at(missingNormals->at(i).vertexIndex).normal, temp);
		XMStoreFloat3(&nList->at(missingNormals->at(i).normalIndex), temp);
	}
}
//temporary sollution
void ObjLoader::generateTangents(vector<MeshVertex> *vertices)
{
	//if(tanList->size() > 0)
	//	tanList->clear();
	//if(bitanList->size() > 0)
	//	bitanList->clear();

	//vector<XMVECTOR>	tan1 = vector<XMVECTOR>();
	//vector<XMVECTOR>	tan2 = vector<XMVECTOR>();

	//for(unsigned int i = 0; i < vList->size(); i++)
	//{
	//	tan1.push_back(XMVECTOR());
	//	tan2.push_back(XMVECTOR());
	//}

	//vector<VertexFaceLink> links = vector<VertexFaceLink>();
	//links.reserve( missingNormals->size() + countedNormals->size() );
	//links.insert(links.end(), missingNormals->begin(), missingNormals->end());
	//links.insert(links.end(), countedNormals->begin(), countedNormals->end());

	//XMVECTOR vec1, vec2;
	//float x1, x2, y1, y2, z1, z2, s1, s2, t1, t2, r, extension;
	//unsigned int i;
	////generate sharp tangents
	//for(i = 0; i < vertices->size(); i+=3)
	//{
	//	x1 = vertices->at(i+1).pos.x - vertices->at(i).pos.x;
	//	x2 = vertices->at(i+2).pos.x - vertices->at(i).pos.x;
	//	y1 = vertices->at(i+1).pos.y - vertices->at(i).pos.y;
	//	y2 = vertices->at(i+2).pos.y - vertices->at(i).pos.y;
	//	z1 = vertices->at(i+1).pos.z - vertices->at(i).pos.z;
	//	z2 = vertices->at(i+2).pos.z - vertices->at(i).pos.z;

	//	s1 = vertices->at(i+1).texC.x - vertices->at(i).texC.x;
	//	s2 = vertices->at(i+2).texC.x - vertices->at(i).texC.x;
	//	t1 = vertices->at(i+1).texC.y - vertices->at(i).texC.y;
	//	t2 = vertices->at(i+2).texC.y - vertices->at(i).texC.y;
	//	extension = s1 * t2 - s2 * t1;
	//	r = 1.0f / extension;

	//	vec1 = XMVector3Normalize( XMLoadFloat3( &XMFLOAT3( (t2 * x1 - t1 * x2) * r,
	//						(t2 * y1 - t1 * y2) * r,
	//						(t2 * z1 - t1 * z2) * r ) ) );


	//	vec2 = XMVector3Normalize( XMLoadFloat3( &XMFLOAT3( (s1 * x2 - s2 * x1) * r,
	//						(s1 * y2 - s2 * y1) * r,
	//						(s1 * z2 - s2 * z1) * r ) ) );

	//	
	//	tan1.at(links.at(i).normalIndex)	= XMVectorAdd(tan1.at(links.at(i).normalIndex), vec1);
	//	tan1.at(links.at(i+1).normalIndex)	= XMVectorAdd(tan1.at(links.at(i+1).normalIndex), vec1);
	//	tan1.at(links.at(i+2).normalIndex)	= XMVectorAdd(tan1.at(links.at(i+2).normalIndex), vec1);

	//	tan2.at(links.at(i).normalIndex)	= XMVectorAdd(tan2.at(links.at(i).normalIndex), vec2);
	//	tan2.at(links.at(i+1).normalIndex)	= XMVectorAdd(tan2.at(links.at(i+1).normalIndex), vec2);
	//	tan2.at(links.at(i+2).normalIndex)	= XMVectorAdd(tan2.at(links.at(i+2).normalIndex), vec2);

	//}

	//XMVECTOR dot, mult, temp;
	//XMFLOAT3 temp2,temp3;
	//for(i = 0; i < vertices->size(); i++)
	//{
	//	//tan1 and tan2 represent tangent together. bitan is not tan2.
	//	//http://answers.unity3d.com/questions/7789/calculating-tangents-vector4.html
	//	//https://devnet.madewithmarmalade.com/questions/4369/tangentbitangent-calculation.html

	//	dot = XMVector3Dot(XMLoadFloat3(&vertices->at(i).normal),tan1.at(links.at(i).normalIndex));
	//	mult = XMVectorMultiply( XMLoadFloat3(&vertices->at(i).normal), dot );

	//	temp = XMVector3Normalize( -( tan1.at(links.at(i).normalIndex), mult ) );
	//	temp2;
	//	XMStoreFloat3(&temp2,temp);

	//	vertices->at(i).tangent.x = temp2.x;
	//	vertices->at(i).tangent.y = temp2.y;
	//	vertices->at(i).tangent.z = temp2.z;
	//	
	//	XMStoreFloat3(&temp3, XMVector3Dot(XMVector3Cross(XMLoadFloat3(&vertices->at(i).normal),tan1.at(links.at(i).normalIndex)), tan2.at(links.at(i).normalIndex)));
	//	if(temp3.x < 0.0f)
	//		vertices->at(i).tangent.w = -1.0f;
	//	else
	//		vertices->at(i).tangent.w = 1.0f;
	//}
}

void ObjLoader::generateTangents2(vector<MeshVertex> *vertices)
{
	if(tanList->size() > 0)
		tanList->clear();
	if(bitanList->size() > 0)
		bitanList->clear();

	vector<XMVECTOR>	tan = vector<XMVECTOR>();
	vector<XMVECTOR>	bitan = vector<XMVECTOR>();

	for(unsigned int i = 0; i < vList->size(); i++)
	{
		tan.push_back(XMVECTOR());
		bitan.push_back(XMVECTOR());
	}

	vector<VertexFaceLink> links = vector<VertexFaceLink>();
	links.reserve( missingNormals->size() + countedNormals->size() );
	links.insert(links.end(), missingNormals->begin(), missingNormals->end());
	links.insert(links.end(), countedNormals->begin(), countedNormals->end());

	float x1, x2, y1, y2, z1, z2, s1, s2, t1, t2, r, extension, length;
	unsigned int i;
	//generate sharp tangents
	for(i = 0; i < vertices->size(); i+=3)
	{
		x1 = vertices->at(i+1).pos.x - vertices->at(i).pos.x;
		x2 = vertices->at(i+2).pos.x - vertices->at(i).pos.x;
		y1 = vertices->at(i+1).pos.y - vertices->at(i).pos.y;
		y2 = vertices->at(i+2).pos.y - vertices->at(i).pos.y;
		z1 = vertices->at(i+1).pos.z - vertices->at(i).pos.z;
		z2 = vertices->at(i+2).pos.z - vertices->at(i).pos.z;

		s1 = vertices->at(i+1).texC.x - vertices->at(i).texC.x;
		s2 = vertices->at(i+2).texC.x - vertices->at(i).texC.x;
		t1 = vertices->at(i+1).texC.y - vertices->at(i).texC.y;
		t2 = vertices->at(i+2).texC.y - vertices->at(i).texC.y;
		extension = s1 * t2 - s2 * t1;
		if(extension == 0)
		{
			tanList->push_back( XMFLOAT3(0,0,0) );
			bitanList->push_back( XMFLOAT3(0,0,0) );
		}
		else
		{
			r = 1.0f / extension;

			tanList->push_back( XMFLOAT3( (t2 * x1 - t1 * x2) * r,
								(t2 * y1 - t1 * y2) * r,
								(t2 * z1 - t1 * z2) * r ) );
			length = sqrt( pow(tanList->at(tanList->size() - 1).x,2) +
							pow(tanList->at(tanList->size() - 1).y,2) +
							pow(tanList->at(tanList->size() - 1).z,2) );
			if(length < 0.0001f)
				tanList->at(tanList->size() - 1) = XMFLOAT3(0,0,0);
			else
				XMStoreFloat3(&tanList->at(tanList->size() - 1), XMVector3Normalize(XMLoadFloat3( &tanList->at(tanList->size() - 1) ) ) );

			bitanList->push_back( XMFLOAT3( (s1 * x2 - s2 * x1) * r,
								(s1 * y2 - s2 * y1) * r,
								(s1 * z2 - s2 * z1) * r ) );
			length = sqrt( pow(bitanList->at(bitanList->size() - 1).x,2) +
							pow(bitanList->at(bitanList->size() - 1).y,2) +
							pow(bitanList->at(bitanList->size() - 1).z,2) );
			if(length < 0.0001f)
				bitanList->at(bitanList->size() - 1) = XMFLOAT3(0,0,0);
			else
				XMStoreFloat3(&bitanList->at(bitanList->size() - 1), XMVector3Normalize(XMLoadFloat3( &bitanList->at(bitanList->size() - 1) ) ) );

		
			/*tan.at(links.at(i).normalIndex)	= XMVectorAdd(tan.at(links.at(i).normalIndex), vec1);
			tan.at(links.at(i+1).normalIndex)	= XMVectorAdd(tan.at(links.at(i+1).normalIndex), vec1);
			tan.at(links.at(i+2).normalIndex)	= XMVectorAdd(tan.at(links.at(i+2).normalIndex), vec1);

			bitan.at(links.at(i).normalIndex)	= XMVectorAdd(bitan.at(links.at(i).normalIndex), vec2);
			bitan.at(links.at(i+1).normalIndex)	= XMVectorAdd(bitan.at(links.at(i+1).normalIndex), vec2);
			bitan.at(links.at(i+2).normalIndex)	= XMVectorAdd(bitan.at(links.at(i+2).normalIndex), vec2); */
		}

	}
	for (int i = 0; i < links.size(); i++)
	{
		vertices->at(links.at(i).vertexIndex).tangent.x += tanList->at(links.at(i).normalIndex).x;
		vertices->at(links.at(i).vertexIndex).tangent.y += tanList->at(links.at(i).normalIndex).y;
		vertices->at(links.at(i).vertexIndex).tangent.z += tanList->at(links.at(i).normalIndex).z;

		vertices->at(links.at(i).vertexIndex).biTangent.x += bitanList->at(links.at(i).normalIndex).x;
		vertices->at(links.at(i).vertexIndex).biTangent.y += bitanList->at(links.at(i).normalIndex).y;
		vertices->at(links.at(i).vertexIndex).biTangent.z += bitanList->at(links.at(i).normalIndex).z;
	}
	for (int i = 0; i < vertices->size(); i++)
	{
		XMStoreFloat3(&vertices->at(i).tangent, XMVector3Normalize(XMLoadFloat3(&vertices->at(i).tangent)));
		XMStoreFloat3(&vertices->at(i).biTangent, XMVector3Normalize(XMLoadFloat3(&vertices->at(i).biTangent)));
	}
}

ObjLoader::~ObjLoader()
{
	SAFE_DELETE( vList );
	SAFE_DELETE( nList );
	SAFE_DELETE( tList );

	SAFE_DELETE( tanList );
	SAFE_DELETE( bitanList );
	SAFE_DELETE( missingNormals );
	SAFE_DELETE( countedNormals );
}
