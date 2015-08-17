cbuffer ConstantBuffer : register(b0)
{
	float4 offset;
}


struct InputVS
{
	float3 pos:IN_POSITION;
	float4 color:IN_COLOR;
};
struct OutputVS
{
	float4 pos:SV_POSITION; // �V�X�e���ɓn�����_�̈ʒu
	float4 color:IN_COLOR;
};

//! ���_�V�F�[�_
OutputVS RenderVS( InputVS inVert )
{
	OutputVS outVert;
	outVert.pos = float4(inVert.pos, 1);
	outVert.color = inVert.color;

	return outVert;
}

struct OutputGS
{
	float4 pos    : SV_POSITION;
	float4 color  : COLOR0;
};

[maxvertexcount(3)]   // �W�I���g���V�F�[�_�[�ŏo�͂���ő咸�_��
void RenderGS(triangle OutputVS In[3],         // �g���C�A���O�� ���X�g���\�����钸�_�z��̓��͏��
	inout TriangleStream<OutputGS> TriStream   // ���_����ǉ����邽�߂̃X�g���[���I�u�W�F�N�g
	)
{
	OutputGS Out;

	int i;

	// ���Ƃ̒��_���o��
	for (i = 0; i < 3; i++)
	{
		Out.pos = In[i].pos + offset;

		Out.color = In[i].color;

		// ���_��ǉ�����
		TriStream.Append(Out);
	}
	// ���݂̃X�g���b�v���I�����A�V�����X�g���b�v���J�n����B
	TriStream.RestartStrip();
}


//! �s�N�Z���V�F�[�_
float4 RenderPS(OutputGS inPixel) : SV_TARGET
{
	//return texDiffuse.Sample(samDiffuse, inPixel.uv);
	return inPixel.color;
}