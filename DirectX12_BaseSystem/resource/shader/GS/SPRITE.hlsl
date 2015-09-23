cbuffer mtxViewport : register(b0)
{
	matrix mtxViewport;
};

/*cbuffer viewport : register(b4)
{
	float viewportDrawable;
	float viewportNum;
};*/

struct OutputVS
{
	float4 pos			: SV_POSITION;
	float4 color		: COLOR0;
	float3 normal       : NORMAL;
	float2 texel        : TEXCOORD0;
};

struct OutputGS
{
float4 pos    : SV_POSITION;
float4 color  : COLOR0;
float3 normal : NORMAL;
float2 texel  : TEXCOORD0;
//uint   ViewportIndex : SV_ViewportArrayIndex;
};

[maxvertexcount(3)]   // �W�I���g���V�F�[�_�[�ŏo�͂���ő咸�_��(viewport * 3�ł悢)
// �W�I���g���V�F�[�_�[
void RenderGS(triangle OutputVS In[3],                    // �g���C�A���O�� ���X�g���\�����钸�_�z��̓��͏��
	inout TriangleStream<OutputGS> TriStream   // ���_����ǉ����邽�߂̃X�g���[���I�u�W�F�N�g
	)
{
	OutputGS Out;

	//int drawable = viewportDrawable;//�r�b�g���Z�̂���int�^�ɃL���X�g
	int i, j;
	//for (j = 0; j < viewportNum; j++)
	//{
	//	if (drawable & 0x01)
	//	{
			// ���Ƃ̒��_���o��
			for (i = 0; i < 3; i++)
			{
				Out.pos = mul(In[i].pos, mtxViewport);

				Out.normal = float3(0.f, 0.f, 0.f);

				Out.texel = In[i].texel;

				Out.color = In[i].color;   // ���̒��_�͕s�����ŏo��

//				Out.ViewportIndex = j;

				// ���_��ǉ�����
				TriStream.Append(Out);
			}
			// ���݂̃X�g���b�v���I�����A�V�����X�g���b�v���J�n����B
			TriStream.RestartStrip();
	//	}
	//	drawable >>= 1;
	//}
}