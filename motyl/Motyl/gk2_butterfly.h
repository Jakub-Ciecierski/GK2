#ifndef __GK2_BUTTERFLY_H_
#define __GK2_BUTTERFLY_H_

#include "gk2_applicationBase.h"
#include "gk2_camera.h"
#include <xnamath.h>

namespace gk2
{
	class Butterfly : public gk2::ApplicationBase
	{
	public:
		Butterfly(HINSTANCE hInstance);
		virtual ~Butterfly();

		//Niektóre struktury xnamath.h wymagaj¹ (np. XMMATRIX czy XMVECTOR) wymagaj¹, aby adresy zmiennych
		//tych typów by³y wyrównane do 16 bajtów. W przypadku dynamicznej alokacji operator new zapewnia tylko
		//wyrównanie do 8 bajtów. Dlatego nale¿y przeci¹¿yæ operator new dla klas, które zawieraj¹
		//pola jednego z tych typów, aby zapewniæ odpowiednie wyrówanie po³o¿enia obiektu w pamiêci.
		static void* operator new(size_t size);
		static void operator delete(void* ptr);

	protected:
		virtual bool LoadContent();
		virtual void UnloadContent();

		virtual void Update(float dt);
		virtual void Render();
	private:
		//radius of a circle inscribed i dodecahedron face
		static const float DODECAHEDRON_R;
		//distance between parallel faces
		static const float DODECAHEDRON_H;
		//diheral angle between to neighbouring faces
		static const float DODECAHEDRON_A;

		//radius of thw Moebius strip
		static const float MOEBIUS_R;
		//width of the Moebius strip
		static const float MOEBIUS_W;
		//Number of parts the Moebius strip is divided to
		static const int MOEBIUS_N;

		//time needed to complete a full lap around the Moebius strip
		static const float LAP_TIME;
		//time of a complete wings flap
		static const float FLAP_TIME;
		//width of a wing
		static const float WING_W;
		//height of a wing
		static const float WING_H;
		//maximum angle between wings
		static const float WING_MAX_A;

		//Various D3D constants
		static const unsigned int VB_STRIDE;
		static const unsigned int VB_OFFSET;
		static const unsigned int BS_MASK;

		//Table of colors for dodecahedron's faces
		static const XMFLOAT4 COLORS[12];

		//Projection matrix
		XMMATRIX m_projMtx;
		//Pentagon -> Dodecahedron's i-th face
		XMMATRIX m_dodecahedronMtx[12];
		//i-th Mirror -> World
		XMMATRIX m_mirrorMtx[12];
		//Butterfly's wings -> World
		XMMATRIX m_wingMtx[2];

		//Camera helper
		gk2::Camera m_camera;

		//Shader's
		std::shared_ptr<ID3D11VertexShader> m_vertexShader;
		std::shared_ptr<ID3D11PixelShader> m_pixelShader;
		//VertexPosNormal input layout
		std::shared_ptr<ID3D11InputLayout> m_inputLayout;

		//Box's vertex buffer
		std::shared_ptr<ID3D11Buffer> m_vbBox;
		//Box's index buffer
		std::shared_ptr<ID3D11Buffer> m_ibBox;
		//Pentagon's vertex buffer
		std::shared_ptr<ID3D11Buffer> m_vbPentagon;
		//Pentagon's index buffer
		std::shared_ptr<ID3D11Buffer> m_ibPentagon;
		//Moebius strip's vertex buffer
		std::shared_ptr<ID3D11Buffer> m_vbMoebius;
		//Moebius strip's index buffer
		std::shared_ptr<ID3D11Buffer> m_ibMoebius;
		//Wing's vertex buffer
		std::shared_ptr<ID3D11Buffer> m_vbWing;
		//Wing's index buffer
		std::shared_ptr<ID3D11Buffer> m_ibWing;

		//Depth stencil state used to fill the stencil buffer
		std::shared_ptr<ID3D11DepthStencilState> m_dssWrite;
		//Depth stencil state used to perform stencil test when drawing mirrored scene
		std::shared_ptr<ID3D11DepthStencilState> m_dssTest;
		//Rasterizer state used to define front faces as counter-clockwise, used when drawing mirrored scene
		std::shared_ptr<ID3D11RasterizerState> m_rsCounterClockwise;
		//Blend state used to draw dodecahedron faced with alpha blending.
		std::shared_ptr<ID3D11BlendState> m_bsAlpha;

		//Shader's constant buffer containing Local -> World matrix
		std::shared_ptr<ID3D11Buffer> m_cbWorld;
		//Shader's constant buffer containing World -> Camera matrix
		std::shared_ptr<ID3D11Buffer> m_cbView;
		//Shader's constant buffer containing projection matrix
		std::shared_ptr<ID3D11Buffer> m_cbProj;
		//Shader's constant buffer containing light's positions
		std::shared_ptr<ID3D11Buffer> m_cbLightPos;
		//Shader's constant buffer containting lighting description
		std::shared_ptr<ID3D11Buffer> m_cbLightColors;
		//Shader's constant buffer containting surface color
		std::shared_ptr<ID3D11Buffer> m_cbSurfaceColor;

		//Path to the shaders' file
		static const std::wstring ShaderFile;

		// Return the position of point on the Moebius strip for parameters t and s
		static XMFLOAT3 MoebiusStripPos(float t, float s);
		// Return the t-derivative of point on the Moebius strip for parameters t and s
		static XMVECTOR MoebiusStripDt(float t, float s);
		// Return the s-derivative of point on the Moebius strip for parameters t and s
		static XMVECTOR MoebiusStripDs(float t, float s);

		//Initializes shaders
		void InitializeShaders();
		//Initializes constant buffers
		void InitializeConstantBuffers();
		//Initializes all render states
		void InitializeRenderStates();
		//Initializes camera
		void InitializeCamera();
		//Initializes vertex and index buffers for the box
		void InitializeBox();
		//Initializes vertex and index buffer for the pentagon
		void InitializePentagon();
		//Initializes matrixes for dodecahedron faces and mirrored scenes
		void InitializeDodecahedron();
		//Initializes vertex and index buffers for the Moebius strip
		void InitializeMoebiusStrip();
		//Initializes vertex and index buffers and matrices for the wings
		void InitializeButterfly();
		//Initializes bilboards
		void InitializeBilboards();

		//Updates camera-related constant buffers
		void UpdateCamera();
		//Updates wing's matrices
		void UpdateButterfly(float dt);

		//Binds shaders to the device context
		void SetShaders();
		//Binds constant buffers to shaders
		void SetConstantBuffers();
		//Sets up one white positional light at the camera position
		void SetLight0();
		//Sets up one white positional light at the camera position and two additional lights, green and blue
		void SetLight1();
		//Sets the value of the surface color constant buffer
		void SetSurfaceColor(const XMFLOAT4& color);

		//Renders a box
		void DrawBox();
		//Renders a dodecahedron with (when drawing alpha blended faces) or without (when drawing mirrored scene) colors
		void DrawDodecahedron(bool colors);
		//Renders a Moebius strip
		void DrawMoebiusStrip();
		//Renders a butterfly
		void DrawButterfly();
		//Renders two bilboards 
		void DrawBilboards();
		//Renders a mirrored scene
		void DrawMirroredWorld(int i);
	};
}

#endif __GK2_BUTTERFLY_H_