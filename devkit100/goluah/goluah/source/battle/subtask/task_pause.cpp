/*----------------------------------------------------------------------

	ポーズ画面

------------------------------------------------------------------------*/

#include "stdafx.h"
#include "task_pause.h"
#include "global.h"

void CTBattlePause::Initialize()
{
	tex_fb = NULL;
	tex_pause = NULL;

	tex_fb = g_draw.GetFrontBufferCopy();
	D3DXCreateTextureFromFileA(g_draw.d3ddev,_T("system\\texture\\pause.png"),&tex_pause);

	m_counter = 0;
	m_face_counter = 0;

	CBattleTaskBase* battleTask = dynamic_cast<CBattleTaskBase*>( g_system.GetCurrentMainTask() );
	if(!battleTask)
	{
		m_face_idx[0] = 0;
		m_face_idx[1] = 0;
	}
	else
	{
		m_face_idx[0] = battleTask->GetActiveCharacterID(0);
		m_face_idx[1] = battleTask->GetActiveCharacterID(1);
	}

	m_kill_flag = FALSE;
	m_inst_on = FALSE;
	ms_inst = NULL;
	m_shiftY = 0;
	m_selected = OPEN_INST;
	m_face_teamid = 0;
}

void CTBattlePause::Terminate()
{
	RELEASE(tex_fb);
	RELEASE(tex_pause);

	g_draw.RelSurface( ms_inst );
}

BOOL CTBattlePause::Execute(DWORD time)
{
	if (m_counter < UINT_MAX)
	{
		m_counter++;
	}

	DWORD key = g_input.GetAllKey();
	if (m_selected == OPEN_INST && key&KEYSTA_BA2)
	{
		m_inst_on = !m_inst_on;
		if(!m_inst_on)
		{
			g_draw.RelSurface( ms_inst );
			ms_inst = NULL;
			m_face_counter = 0;
		}
		else
		{
			m_face_teamid = 0;
			m_face_idx[m_face_teamid] = 0;
			ChangeInst();
		}
	}
	else if (m_selected == CONTINUE_BATTLE && key & KEYSTA_BA2)
	{//試合再開
		Kill();
	}
	else if (m_selected == RETURN_TO_TITLE && key&KEYSTA_BA2)
	{//タイトルに戻る
		g_system.ReturnTitle();
		return FALSE;
	}
	else if (key & KEYSTA_DOWN2)
	{
		if (m_selected == RETURN_TO_TITLE)m_selected = CONTINUE_BATTLE;
		else m_selected++;
	}
	else if (key & KEYSTA_UP2)
	{
		if (m_selected == CONTINUE_BATTLE)m_selected = RETURN_TO_TITLE;
		else m_selected--;
	}

	if(m_inst_on)
	{
		if (m_face_counter < UINT_MAX)
		{
			m_face_counter++;
		}

		if (key & KEYSTA_RIGHT2)
		{
			if (m_face_idx[m_face_teamid] == (g_battleinfo.GetNumTeam(m_face_teamid) - 1))
			{
				m_face_teamid = (m_face_teamid == 0) ? 1 : 0;
				m_face_idx[m_face_teamid] = 0;
			}
			else
			{
				m_face_idx[m_face_teamid]++;
			}
			m_face_counter = 0;
			m_shiftY = 0;
			ChangeInst();
		}
		else if (key & KEYSTA_LEFT2)
		{
			if (m_face_idx[m_face_teamid] == 0)
			{
				m_face_teamid = (m_face_teamid == 0) ? 1 : 0;
				m_face_idx[m_face_teamid] = (g_battleinfo.GetNumTeam(m_face_teamid) - 1);
			}
			else
			{
				m_face_idx[m_face_teamid]--;
			}
			m_face_counter = 0;
			m_shiftY = 0;
			ChangeInst();
		}
		else if (key & KEYSTA_UP)
		{
			m_shiftY++;
		}
		else if (key & KEYSTA_DOWN)
		{
			m_shiftY--;
		}

		if (key & KEYSTA_BD2)
		{
			m_shiftY = 0;
		}
	}

	return m_kill_flag ? FALSE : TRUE;
}

void CTBattlePause::Draw()
{
	g_draw.EnableZ(FALSE,FALSE);
	g_draw.SetTransform(FALSE);
	g_draw.d3ddev->SetVertexShader(FVF_3DVERTEX);

	const float ar = 320.0f/240.0f;
	const float adj = 0.004f;

	MYVERTEX3D vb[4];

	vb[0].z = 0.0f;
	vb[1].z = 0.0f;
	vb[2].z = 0.0f;
	vb[3].z = 0.0f;

	vb[0].tu = 0.0f;
	vb[1].tu = 0.0f;
	vb[2].tu = 1.0f;
	vb[3].tu = 1.0f;
	
	vb[0].tv = 0.0f;
	vb[1].tv = 1.0f;
	vb[2].tv = 0.0f;
	vb[3].tv = 1.0f;

	//フロントバッファをコピーしたやつ
	if(tex_fb)
	{
		DWORD col;
		
		col = 255 - (m_counter>100 ? 100 : m_counter);
		vb[0].color = 
		vb[1].color = 0xFF000000 | (col<<16) | (col<<8) | col;

		col = 255 - (m_counter>100 ? 100 : m_counter);
		vb[2].color = 
		vb[3].color = 0xFF000000 | (col<<16) | (col<<8) | col;

		vb[0].x =  adj;
		vb[1].x =  adj;
		vb[2].x =  (2.0f-adj)*ar;
		vb[3].x =  (2.0f-adj)*ar;

		vb[0].y =  0.0f;
		vb[1].y =  (2.0f-adj);
		vb[2].y =  0.0f;
		vb[3].y =  (2.0f-adj);

		g_draw.d3ddev->SetTexture(0,tex_fb);
		g_draw.d3ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,vb,sizeof(MYVERTEX3D));
	}
	
	if (!m_inst_on)
	{
		//"Pause - Press F7 Key"
		g_draw.SetAlphaMode(GBLEND_KASAN);
		if (tex_pause)
		{
			vb[0].color =
			vb[1].color =
			vb[2].color =
			vb[3].color = 0xFF55FF33;

			vb[0].x = (320.0f - 144.0f) / 240.0f;
			vb[1].x = (320.0f - 144.0f) / 240.0f;
			vb[2].x = (320.0f + 144.0f) / 240.0f;
			vb[3].x = (320.0f + 144.0f) / 240.0f;

			vb[0].y = 50.0f / 240.0f;
			vb[1].y = (50.0f + 91.0f) / 240.0f;
			vb[2].y = 50.0f / 240.0f;
			vb[3].y = (50.0f + 91.0f) / 240.0f;

			g_draw.d3ddev->SetTexture(0, tex_pause);
			g_draw.d3ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vb, sizeof(MYVERTEX3D));
		}
		g_draw.SetAlphaMode(0);

		DWORD text_color;
		const DWORD text2_color = 0xFF4455AA;//選択されているときの色
		const DWORD text1_color = 0xFF55FF33;//選択されていないときの色
		DWORD top_y = 300;
		DWORD step_y = 30;

		if (m_selected == CONTINUE_BATTLE)text_color = text1_color;
		else text_color = text2_color;
		g_system.DrawBMPText(320 - 68, top_y, 0, _T("CONTINUE"), text_color);
		top_y += step_y;

		if (m_selected == OPEN_INST)text_color = text1_color;
		else text_color = text2_color;
		g_system.DrawBMPText(320 - 68, top_y, 0, _T("OPEN INST"), text_color);
		top_y += step_y;

		if (m_selected == RETURN_TO_TITLE)text_color = text1_color;
		else text_color = text2_color;
		g_system.DrawBMPText(320 - 113, top_y, 0, _T("RETURN TO TITLE"), text_color);
		top_y += step_y;

		return;
	}

	int alt;
	MYSURFACE *dds_face;
	RECT r_face;
	int x,y;

	//デカ顔

	alt = OPT2ALT(g_battleinfo.GetCharacterOption(m_face_teamid, m_face_idx[m_face_teamid]));
	dds_face = gbl.GetBigFace(g_battleinfo.GetCharacter(m_face_teamid, m_face_idx[m_face_teamid]),g_battleinfo.GetColor(m_face_teamid, m_face_idx[m_face_teamid]),alt);

	if (dds_face)
	{
		r_face.top = 0;
		r_face.left = 0;
		r_face.right = (int)dds_face->wg;
		r_face.bottom = (int)dds_face->hg;

		if(m_face_teamid ==0){
			x = m_face_counter*30 - (int)dds_face->wg;
			if(x>0)x=0;
		}
		else{
			x = 640 - m_face_counter*30;
			if(x< 640-(int)dds_face->wg)x=640-(int)dds_face->wg;
		}

		g_draw.CheckBlt(
					dds_face,
					x,
					240-(DWORD)dds_face->hg/2,
					r_face,
					m_face_teamid == 0 ? FALSE : TRUE,
					FALSE,
					0,
					-0.01f,
					0xFFFFFFFF);
		//inst
		if(ms_inst)
		{
			r_face.top = 0;
			r_face.left = 0;
			r_face.right = (int)ms_inst->wg;
			r_face.bottom = (int)ms_inst->hg;

			DWORD alpha ;

			x = m_face_teamid ==0 ? 20 : 620-(int)ms_inst->wg;
			y = 450-(DWORD)ms_inst->hg+m_shiftY;

			//下地
			int mgn = 0;
			vb[0].x =  (x-mgn)/240.0f;
			vb[1].x =  (x-mgn)/240.0f;
			vb[2].x =  (x+mgn+ms_inst->wg)/240.0f;
			vb[3].x =  (x+mgn+ms_inst->wg)/240.0f;

			vb[0].y =  (y-mgn)/240.0f;
			vb[1].y =  (y+mgn+ms_inst->hg)/240.0f;
			vb[2].y =  (y-mgn)/240.0f;
			vb[3].y =  (y+mgn+ms_inst->hg)/240.0f;
			
			vb[0].z = 
			vb[1].z = 
			vb[2].z = 
			vb[3].z = -0.02f;
			
			alpha = m_face_counter*10>220 ? 220 : m_face_counter*10;

			vb[0].color = 
			vb[1].color = 
			vb[2].color = 
			vb[3].color = alpha<<24 | 0x00FFFFFF;
			
			D3DXMATRIX mat;
			D3DXMatrixIdentity(&mat);
			g_draw.d3ddev->SetTransform(D3DTS_WORLD,&mat);

			g_draw.d3ddev->SetTexture(0,NULL);
			g_draw.d3ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,vb,sizeof(MYVERTEX3D));

			//ビットマップ
			alpha = m_face_counter*10>255 ? 255 : m_face_counter*10;
			g_draw.CheckBlt(
					ms_inst,
					x,
					y,
					r_face,
					FALSE,
					FALSE,
					0,
					-0.03f,
					alpha<<24 | 0x00FFFFFF );
		}
	}
}


void CTBattlePause::ChangeInst()
{
	UINT alt = OPT2ALT(g_battleinfo.GetCharacterOption(m_face_teamid, m_face_idx[m_face_teamid]));
	UINT char_index = g_battleinfo.GetCharacter(m_face_teamid, m_face_idx[m_face_teamid]);
	
	ms_inst = gbl.GetInst(char_index, alt);
}

