#pragma once
namespace Maths  {
	inline float Scalar(Vector3 a, Vector3 b) {
		return (a.x*b.x)+(a.y*b.y)+(a.z*b.z);
	}

	inline float GetVecNorm(Vector3 vec) {
		return sqrt(pow(vec.x,2)+pow(vec.y,2)+pow(vec.z,2));	
	}

	inline float Get3Distance(Vector3 a, Vector3 b) {
		Vector3 TraceLine = {b.x-a.x,b.y-a.y,b.z-a.z};

		float distance = GetVecNorm(TraceLine);


		return distance < 0 ? -distance : distance; 
	}

	inline void NormalizeVec(Vector3& vec, Vector3& out) {
		float norm = GetVecNorm(vec);

		out.x = vec.x / norm;
		out.y = vec.y / norm;
		out.z = vec.z / norm;
	}

	//Game uses Direct3D -> check loaded modules
	bool WorldToScreen(Vector3 pos, Vector2 &screen, Mat4x4& matrix, int windowWidth, int windowHeight)
	{
		//Matrix multiplication between 3D world space coords and view projection matrix
		//screen.x = matrix.m[0][0] * pos.x + matrix.m[1][0] * pos.y + matrix.m[2][0] * pos.z + 1*matrix.m[3][0];
		screen.x = matrix.m[0][0] * pos.x + matrix.m[0][1] * pos.y + matrix.m[0][2] * pos.z + 1*matrix.m[0][3];

		//screen.y = matrix.m[0][1] * pos.x + matrix.m[1][1] * pos.y + matrix.m[2][1] * pos.z + 1*matrix.m[3][1];
		screen.y = matrix.m[1][0] * pos.x + matrix.m[1][1] * pos.y + matrix.m[1][2] * pos.z + 1*matrix.m[1][3];

		//We ignore the z coord since we don't use it in the end

		//float w = matrix.m[0][3] * pos.x + matrix.m[1][3] * pos.y + matrix.m[2][3] * pos.z + 1 * matrix.m[3][3];
		float w = matrix.m[3][0] * pos.x + matrix.m[3][1] * pos.y + matrix.m[3][2] * pos.z + 1 * matrix.m[3][3];
;		
		//Depth Test
		if (w < 0.1f)
			return false;

		Vector2 NDC;//normalized device coordinates
		NDC.x = screen.x / w;
		NDC.y =	screen.y / w;
		//NDC.z ignored since we don't use it for our calculations

		//Normalizing screen axes to go from -1 to 1 on both x and y
		screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
		screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
		
		return true;
	}

};
