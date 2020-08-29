/*
 * rk_aiq_algo_afec_genmesh.h
 *
 *  Copyright (c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _RK_AIQ_ALGO_AFEC_GENMESH_H_
#define _RK_AIQ_ALGO_AFEC_GENMESH_H_


/* ���۲��� */
struct CameraCoeff
{
	double cx, cy;					// �����ͷ�Ĺ���
	double a0, a2, a3, a4;			// ���۾�ͷ�Ļ���ϵ��
	double c, d, e;					// �ڲ�[c d;e 1]
	double sf;						// sf�����ӽǣ�sfԽ���ӽ�Խ��
	int invpolNum;					// rho-theta����ʽ��ϵ�ϵ������
	double invpol[21];				// rho-theta����ʽ��ϵ�ϵ������ߴ���20��
	double big_rho[2002];			// Ԥ�����ɵ�tan(theta)��rho�Ķ�Ӧ��
	double small_rho[2001];			// Ԥ�����ɵ�cot(theta)��rho�Ķ�Ӧ��
	double Z[5000];
};

/* ����ͼ����������ֱ��ʣ�����FECУ�������ز��� */
void calcFecMeshParams(int srcW, int srcH, int dstW, int dstH, int *meshSizeX, int *meshSizeY, int *meshStepW, int *meshStepH);

/* FEC mesh �ڴ����� */
unsigned long mallocFecMesh(int srcWidth, int srcHeight, int meshStepW, int meshStepH, unsigned short **pMeshXI, unsigned char **pMeshXF, unsigned short **pMeshYI, unsigned char **pMeshYF);

/* FEC mesh �ڴ��ͷ� */
void freeFecMesh(unsigned short *pMeshXI, unsigned char *pMeshXF, unsigned short *pMeshYI, unsigned char *pMeshYF);

/*************************************************************
�������ܣ����ɲ�ͬУ���̶ȵ�meshX������ISP��FECģ��

	���룺
	1��int srcWidth, int srcHeight
	2������궨����:
			camCoeff.cx;
			camCoeff.cy;
			camCoeff.a0;
			camCoeff.a2;
			camCoeff.a3;
			camCoeff.a4;
	3����ҪУ���ĳ̶�
			level(0-255: 0��ʾУ���̶�Ϊ0%, 255��ʾУ���̶�Ϊ100%)

	�����pMeshXI, pMeshXF, pMeshYI, pMeshYF
**************************************************************/
bool genFECMeshNLevel(int srcWidth, int srcHeight, CameraCoeff &camCoeff, int level, unsigned short *pMeshXI, unsigned char *pMeshXF, unsigned short *pMeshYI, unsigned char *pMeshYF);
#endif // !_RK_AIQ_ALGO_AFEC_GENMESH_H_
