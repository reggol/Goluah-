﻿/*=========================================================================
	
	戦闘タスク基礎

	ネットワーク対応・ネットワーク非対応 の両方の戦闘タスクの共通部分

===========================================================================*/

#include "stdafx.h"
#include "define_const.h"
#include "define_macro.h"
#include "global.h"
#include "battleTaskBase.h"
#include "task_loading.h"

#include "adaptor\define_data.h"	// 消失・復活
#include "adaptor\define_char.h"
#include "adaptor\mimikaki.h"
#include "adaptor\adaptor.h"


CBattleTaskBase::CBattleTaskBase()
{
	int i,j;
	for(j=0;j<2;j++){
		for(i=0;i<MAXNUM_TEAM;i++){
			hlib_c[j][i]=NULL;
		}
	}
	hlib_s=NULL;
	dsb_round = NULL;
	dsb_fight = NULL;
	dsb_ko = NULL;
}

/*==========================================================================


	タスク Initialize


============================================================================*/

void CBattleTaskBase::Initialize()
{
	g_system.PushSysTag(__FUNCTION__);

	//[ FIGHT / KO ]のサウンドをロード
	dsb_fight = g_sound.CreateDSB(_T(".\\system\\sound\\fight.wav"));
	dsb_ko = g_sound.CreateDSB(_T(".\\system\\sound\\ko.wav"));
	dsb_timeover = g_sound.CreateDSB(_T(".\\system\\sound\\timeover.wav"));
	dsb_timeover1 = g_sound.CreateDSB(_T(".\\system\\sound\\timeover1.wav"));
	dsb_timeover2 = g_sound.CreateDSB(_T(".\\system\\sound\\timeover2.wav"));

	InitializeParameters();
	InitializeObjectList();
	InitializeDLLLoadInfo();
	InitializeLoadDLLs();
	InitializeSubTasks();

	g_system.PopSysTag();
}


//DLLに渡す構造体のパラメータを初期化する
void CBattleTaskBase::InitializeDLLLoadInfo()
{
	UINT i,j;

	for(j=0;j<2;j++)
	{
		for(i=0;i<g_battleinfo.GetNumTeam(j);i++){
			_tcscpy(m_cinfo[j][i].dir,g_charlist.GetCharacterDir(g_battleinfo.GetCharacter(j,i)));
			m_cinfo[j][i].tid = j;
			m_cinfo[j][i].funcs = &g_exp.fpack_s;
			m_cinfo[j][i].funco = &g_exp.fpack_o;
			m_cinfo[j][i].funcd = &g_exp.fpack_d;
			m_cinfo[j][i].color = g_battleinfo.GetColor(j,i);
			m_cinfo[j][i].keyinput = g_battleinfo.GetKeyAssign(j,i);
			m_cinfo[j][i].options_flag = g_battleinfo.GetCharacterOption(j,i);
		}
	}

	m_sinfo.funcs = &g_exp.fpack_s;
	m_sinfo.funco = &g_exp.fpack_o;
	m_sinfo.funcd = &g_exp.fpack_d;
	m_sinfo.tid = TEAM_STAGE;
	_tcscpy(m_sinfo.dir,g_stagelist.GetStageDir(g_battleinfo.GetStage()));
}


void CBattleTaskBase::InitializeLoadDLLs()
{
	int i,j;
	TCHAR filename [MAX_PATH];

	CTNowLoading* now_loading = dynamic_cast<CTNowLoading*>(g_system.FindTask('LOAD'));

	//キャラクターdllのロード
	PFUNC_CREATECHARACTER pf_create=NULL;

	ZeroMemory(charobjid, sizeof(charobjid));
	stgobjid = 0;
	for(j=0;j<2;j++)
	{
		switch(g_battleinfo.GetNumTeam(j))
		{
		case 3://3人目をロードする
			i=2;
			m_crnt_dllid = j*MAXNUM_TEAM + i+1;
			m_current_com_level = g_battleinfo.GetComLevel(j,i);
			if ( g_charlist.GetCharacterVer(g_battleinfo.GetCharacter(j, i)) < 680 )
			{
				// アダプターが必要（消失）
				CCharAdaptor* pca = new CCharAdaptor;

				hlib_c[j][i] = NULL;
				if (pca)
				{
					if ( (charobjid[j][i] = pca->CreateCharacter(&m_cinfo[j][i])) == 0)
						{ delete pca; return; }
				}
				else {return;}
			}
			else
			{
				_stprintf(filename,_T("%s\\action.dll"),
					g_charlist.GetCharacterDir(g_battleinfo.GetCharacter(j,i)));
				hlib_c[j][i] = LoadLibrary(filename);

				if(hlib_c[j][i] != NULL){
					pf_create = (PFUNC_CREATECHARACTER)GetProcAddress(hlib_c[j][i],_T("CreateCharacter"));
					if(pf_create!=NULL){
						charobjid[j][i] = (*pf_create)(&m_cinfo[j][i]);
					}
					else {return;}
				}
				else {return;}
			}
			if(now_loading)now_loading->Proceed(NowLoading_DLL);
		case 2://2人目をロードする
			i=1;
			m_crnt_dllid = j*MAXNUM_TEAM + i+1;
			m_current_com_level = g_battleinfo.GetComLevel(j,i);
			if ( g_charlist.GetCharacterVer(g_battleinfo.GetCharacter(j, i)) < 680 )
			{
				// アダプターが必要（消失）
				CCharAdaptor* pca = new CCharAdaptor;

				hlib_c[j][i] = NULL;
				if (pca)
				{
					if ( (charobjid[j][i] = pca->CreateCharacter(&m_cinfo[j][i])) == 0)
						{ delete pca; return; }
				}
				else {return;}
			}
			else
			{
				_stprintf(filename,_T("%s\\action.dll"),
					g_charlist.GetCharacterDir(g_battleinfo.GetCharacter(j,i)));
				hlib_c[j][i] = LoadLibrary(filename);

				if(hlib_c[j][i] != NULL){
					pf_create = (PFUNC_CREATECHARACTER)GetProcAddress(hlib_c[j][i],_T("CreateCharacter"));
					if(pf_create!=NULL){
						charobjid[j][i] = (*pf_create)(&m_cinfo[j][i]);
					}
					else {return;}
				}
				else {return;}
			}
			if(now_loading)now_loading->Proceed(NowLoading_DLL);
		case 1://1人目をロードする
			i=0;
			m_crnt_dllid = j*MAXNUM_TEAM + i+1;
			m_current_com_level = g_battleinfo.GetComLevel(j,i);
			if ( g_charlist.GetCharacterVer(g_battleinfo.GetCharacter(j, i)) < 680 )
			{
				// アダプターが必要（消失）
				CCharAdaptor* pca = new CCharAdaptor;

				hlib_c[j][i] = NULL;
				if (pca)
				{
					if ( (charobjid[j][i] = pca->CreateCharacter(&m_cinfo[j][i])) == 0)
						{ delete pca; return; }
				}
				else {return;}
			}
			else
			{
				_stprintf(filename,_T("%s\\action.dll"),
					g_charlist.GetCharacterDir(g_battleinfo.GetCharacter(j,i)));
				hlib_c[j][i] = LoadLibrary(filename);

				if(hlib_c[j][i] != NULL){
					pf_create = (PFUNC_CREATECHARACTER)GetProcAddress(hlib_c[j][i],_T("CreateCharacter"));
					if(pf_create!=NULL){
						charobjid[j][i] = (*pf_create)(&m_cinfo[j][i]);
					}
					else {return;}
				}
				else {return;}
			}
			if(now_loading)now_loading->Proceed(NowLoading_DLL);
		}
	}
	m_current_com_level = g_config.GetDifficulty();

	//ステージDLLのロード
	m_crnt_dllid = 2*MAXNUM_TEAM+1;
	_stprintf(filename,_T("%s\\stage.dll"),
		g_stagelist.GetStageDir(g_battleinfo.GetStage()));
	hlib_s = LoadLibrary(filename);
	if(hlib_s == NULL){
		hlib_s = LoadLibrary(_T(".\\system\\defstg.dll"));
		if(!hlib_s){
			g_system.Log(_T("failed to load default stage DLL"),SYSLOG_WARNING);
		}
		if(now_loading)now_loading->Proceed(NowLoading_DLL);
	}
	PFUNC_CREATESTAGE pf_create_s=NULL;
	pf_create_s = (PFUNC_CREATESTAGE)GetProcAddress(hlib_s,_T("CreateStage"));
	if(pf_create_s!=NULL){
		stgobjid = (*pf_create_s)(&m_sinfo);
	}
	else{
		g_system.Log(_T("failed to get stage DLL create function pointer"),SYSLOG_WARNING);
		g_system.Log(g_stagelist.GetStageDir(g_battleinfo.GetStage()),SYSLOG_WARNING);
	}

	m_crnt_dllid = 0;

	gbl.ods(_T("CBattleTaskBase::InitializeLoadDLLs , DLL Load Complete"));
}



/*==========================================================================


	タスク Terminate


============================================================================*/
void CBattleTaskBase::Terminate()
{
	g_system.PushSysTag(__FUNCTION__);

	TerminateObjectList();
	TerminateUnloadDLLs();
	TerminateDestroySubTasks();

	g_input.KeyLock(FALSE);

	g_system.PopSysTag();
}

//DLL開放
void CBattleTaskBase::TerminateUnloadDLLs()
{
	g_system.PushSysTag(__FUNCTION__);

	int i,j/*,k,l*/;
	for(j=0;j<2;j++){
		for(i=0;i<MAXNUM_TEAM;i++){
			//dllの開放（重複注意）
			/*for(k=0;k<2;k++){
				for(l=0;l<MAXNUM_TEAM;l++){
					if(hlib_c[j][i]!=hlib_c[k][l]){*/	// 重複は参照カウントでどうにかしてるみたい
						RELEASEDLL(hlib_c[j][i]);
					/*}
					else
						hlib_c[j][i]=NULL;
				}
			}*/
		}
	}
	RELEASEDLL(hlib_s);

	for(j=0;j<2;j++){
		for(i=0;i<MAXNUM_TEAM;i++){//idを0にしておく。0=オブジェクト無し
			charobjid[j][i]=0;
		}
	}
	stgobjid=0;

	RELEASE(dsb_round);
	RELEASE(dsb_fight);
	RELEASE(dsb_ko);

	g_system.PopSysTag();
}




/*==========================================================================


	オブジェクトサービス


============================================================================*/

/*--------------------------------------------------------------------------
	強制ダメージ
		投げ等で使用される。このダメージではKO判定は行われない。
		投げダメージで死に至った場合は、投げの開放時に死亡判定が行われる。
		（投げ中でないときはKO判定を行うようにしてみました。）
----------------------------------------------------------------------------*/
void CBattleTaskBase::AddDamage(DWORD oid,DWORD eoid,int x,int y)
{
	g_system.PushSysTag(__FUNCTION__);

	if(bf_state!=BFSTATE_FIGHTING){
		g_system.PopSysTag();
		return;
	}
	DWORD a_id = oid;
	DWORD k_id = eoid;

	CGObject *attacker=GetGObject(a_id);
	CGObject *higaisya=GetGObject(k_id);

	if(attacker==NULL || higaisya==NULL){
		g_system.PopSysTag();
		return;
	}

	attacker->Message(GOBJMSG_TOUCHC,eoid);//とりあえず当たったことを通知

	//喰らったダメージ情報をコピー
	higaisya->data.atk2.info1 = attacker->data.atk;
	higaisya->data.atk2.oid = a_id;
	//フラグ立
	higaisya->data.atk2.flags = 0;
	//画面端で相手を押し戻すかどうか
	if(higaisya->data.objtype & GOBJFLG_CLIPX){
		if(attacker->data.objtype & GOBJFLG_HANSAYOU){
			higaisya->data.atk2.flags |= ATKINFO2_ATTACKERBACK;
		}
	}
	//のけぞりの向き
	if(attacker->data.muki){
		if(attacker->data.atk->muki){
			higaisya->data.atk2.flags |= ATKINFO2_RIGHTBACK;
		}
	}
	else{
		if(!(attacker->data.atk->muki)){
			higaisya->data.atk2.flags |= ATKINFO2_RIGHTBACK;
		}
	}

	DWORD i;
	GOBJECT		*pdat  =&(higaisya->data);
	ATTACKINFO  *aif = attacker->data.atk;
	MY2DVECTOR kas_point;
	kas_point.x =x;
	kas_point.y =y;

	double dmkanwa;
	if(TRUE){//常に喰らう
		dmkanwa=1.0;//ダメージ緩和量
		//hit count
		if(pdat->aid&ACTID_KURAI){
			higaisya->hitcount++;
			if(higaisya->data.id == charobjid[higaisya->data.tid][active_character[higaisya->data.tid]]){//連続ヒット表示
				if(higaisya->hitcount==2)
					bf_hitdisp[pdat->tid]=0;
				else bf_hitdisp[pdat->tid]=30;
			}
		}
		else{
			higaisya->hitcount=1;
			if(higaisya->data.id == charobjid[higaisya->data.tid][active_character[higaisya->data.tid]]){//連続ヒット表示
				bf_hitdisp[pdat->tid]=0;
			}
		}
		for(i=1;i<higaisya->hitcount;i++){
			dmkanwa*=RDMGKANWA;
		}
		if(dmkanwa<DMGKANWA_MINIMUM)dmkanwa=DMGKANWA_MINIMUM;

		if(bf_state==BFSTATE_FIGHTING)pdat->hp -= (DWORD)(pdat->atk2.info1->damage * dmkanwa);
		if(higaisya->hitcount == 1)
			higaisya->sexydamage  = (DWORD)(pdat->atk2.info1->damage * dmkanwa);
		else if(higaisya->hitcount>=2){
			higaisya->sexydamage += (DWORD)(pdat->atk2.info1->damage * dmkanwa);

			if(higaisya->hitcount == 2)		// 表示用も設定
				higaisya->sexydamage_anim = higaisya->sexydamage;
			else							// 幅設定
				higaisya->sexydamage_haba = abs((int)higaisya->sexydamage - (int)higaisya->sexydamage_anim) / 8;
		}

		switch(aif->hit & 0x000F0000){//ヒットマーク描画
		case 0:break;
		case HITINFO_MARK1:AddEffect(EFCTID_MARK1,(int)kas_point.x,(int)kas_point.y);break;
		case HITINFO_MARK2:AddEffect(EFCTID_MARK2,(int)kas_point.x,(int)kas_point.y);break;
		case HITINFO_MARK3:AddEffect(EFCTID_MARK3,(int)kas_point.x,(int)kas_point.y);break;
		case HITINFO_MARK4:AddEffect(EFCTID_MARK4,(int)kas_point.x,(int)kas_point.y);break;
		}
		switch(aif->hit & 0x00F00000){//効果音
		case 0:break;
		case HITINFO_SNDHIT1:g_system.PlaySystemSound(SYSTEMSOUND_HIT1);break;
		case HITINFO_SNDHIT2:g_system.PlaySystemSound(SYSTEMSOUND_HIT2);break;
		case HITINFO_SNDHIT3:g_system.PlaySystemSound(SYSTEMSOUND_HIT3);break;
		case HITINFO_SNDSHK1:g_system.PlaySystemSound(SYSTEMSOUND_SHOCK1);break;
		case HITINFO_SNDSHK2:g_system.PlaySystemSound(SYSTEMSOUND_SHOCK2);break;
		default:CSystem::Log(_T("実装されていないヒット効果音が指定された"),SYSLOG_WARNING);
		}
		switch(aif->hit & 0x0F000000){//ヒットストップ
		case 0:break;
		case HITINFO_SIV1:HitStop( 3,k_id);break;
		case HITINFO_SIV2:HitStop( 5,k_id);break;
		case HITINFO_SIV3:HitStop(10,k_id);break;
		case HITINFO_STOP:HitStop(40,k_id);break;
		default:CSystem::Log(_T("実装されていないヒットストップID"),SYSLOG_WARNING);
		}
		if(pdat->aid != ACTID_NAGERARE && pdat->hp<=0){//死亡
			if(g_battleinfo.GetBattleType()==TAISENKEISIKI_GOCYAMAZE)
				pdat->aid=ACTID_FINALDOWN;
			else
				pdat->aid=ACTID_KAITENFINISH;
			if(aif->hit & HITINFO_EFCTBURN )AddEffect(EFCTID_BURN,0,0,k_id);
			if(aif->hit & HITINFO_EFCTBURN_B )AddEffect(EFCTID_BURN_B,0,0,k_id);
			if(aif->hit & HITINFO_EFCTBURN_G )AddEffect(EFCTID_BURN_G,0,0,k_id);
		}
		//エフェクト
		if((aif->hit & 0x0000F000) & HITINFO_EFCTSINDO)AddEffect(EFCTID_SINDO,2,20);
		if((aif->hit & 0x0000F000) & HITINFO_EFCTBURN  )AddEffect(EFCTID_BURN  ,0,0,k_id);
		if((aif->hit & 0x0000F000) & HITINFO_EFCTBURN_B)AddEffect(EFCTID_BURN_B,0,0,k_id);
		if((aif->hit & 0x0000F000) & HITINFO_EFCTBURN_G)AddEffect(EFCTID_BURN_G,0,0,k_id);
		
		//攻撃が当たったことを攻撃したやつに通知
		attacker->Message(GOBJMSG_TOUCHB,TRUE);
	}
	g_system.PopSysTag();
}



/*--------------------------------------------------------------------------
	間合い取得
	×両者の回転を考慮していない
	×オブジェクトの座標指定がディスプレイ座標だった場合を考慮していない
----------------------------------------------------------------------------*/

//↓間合い取得関数 に必要
inline void koukan(int *a,int *b)
{
	int tmp=*a;
	*a=*b;
	*b=tmp;
}

DWORD CBattleTaskBase::GetMaai(DWORD oid,DWORD eoid,BOOL h)
{
	GOBJECT *pdat = (GOBJECT*)GetGObjectInfo(oid);
	GOBJECT *pedat = (GOBJECT*)GetGObjectInfo(eoid);
	if(pdat==NULL)return(99999);
	if(pdat->phdat==NULL)return(99999);
	if(pedat==NULL)return(99999);
	if(pedat->phdat==NULL)return(99999);

	int i;
	int min_p1=99999,max_p1=-99999,min_p2=99999,max_p2=-99999;
	int hm2=99999,vm2=99999;

	if(!h){//水平間合い
		for(i=0;i<MAXNUM_TEAM;i++){
			if(min_p1 > pdat->phdat[ pdat->cnow ].kas[i].left)
				min_p1 = pdat->phdat[ pdat->cnow ].kas[i].left;
			if(max_p1 < pdat->phdat[ pdat->cnow ].kas[i].right)
				max_p1 = pdat->phdat[ pdat->cnow ].kas[i].right;
			if(min_p2 > pedat->phdat[ pedat->cnow ].kas[i].left)
				min_p2 = pedat->phdat[ pedat->cnow ].kas[i].left;
			if(max_p2 < pedat->phdat[ pedat->cnow ].kas[i].right)
				max_p2 = pedat->phdat[ pedat->cnow ].kas[i].right;
		}
		if(max_p1<min_p1)koukan(&min_p1,&max_p1);
		if(max_p2<min_p2)koukan(&min_p2,&max_p2);
		min_p1 += (int)pdat->x;
		max_p1 += (int)pdat->x;
		min_p2 += (int)pedat->x;
		max_p2 += (int)pedat->x;

		if(pdat->x > pedat->x)hm2 = min_p1 - max_p2;
		else                  hm2 = min_p2 - max_p1;
		if(hm2<0)hm2=0;
		return(hm2);
	}
	else {//垂直間合い
		for(i=0;i<MAXNUM_TEAM;i++){
			if(min_p1 > pdat->phdat[ pdat->cnow ].kas[i].top)
				min_p1 = pdat->phdat[ pdat->cnow ].kas[i].top;
			if(max_p1 < pdat->phdat[ pdat->cnow ].kas[i].bottom)
				max_p1 = pdat->phdat[ pdat->cnow ].kas[i].bottom;
			if(min_p2 > pedat->phdat[ pedat->cnow ].kas[i].top)
				min_p2 = pedat->phdat[ pedat->cnow ].kas[i].top;
			if(max_p2 < pedat->phdat[ pedat->cnow ].kas[i].bottom)
				max_p2 = pedat->phdat[ pedat->cnow ].kas[i].bottom;
		}
		if(max_p1<min_p1)koukan(&min_p1,&max_p1);
		if(max_p2<min_p2)koukan(&min_p2,&max_p2);
		min_p1 += (int)pdat->y;
		max_p1 += (int)pdat->y;
		min_p2 += (int)pedat->y;
		max_p2 += (int)pedat->y;
		if(pdat->y > pedat->y)vm2 = min_p1 - max_p2;
		else                  vm2 = min_p2 - max_p1;
		if(vm2<0)vm2=0;
		return(vm2);
	}
}



/*--------------------------------------------------------------------------
	オブジェクトIDから、オブジェクト情報取得
----------------------------------------------------------------------------*/
GOBJECT* CBattleTaskBase::GetGObjectInfo(DWORD oid)
{
	CGObject *pgobj = GetGObject(oid);
	if(pgobj==NULL)return(NULL);
	else return( &pgobj->data );
}

/*--------------------------------------------------------------------------
	チームID,キャラクタインデックスから、オブジェクト情報取得
----------------------------------------------------------------------------*/
GOBJECT* CBattleTaskBase::GetCharacterInfo(DWORD j,DWORD i)
{
	CGObject *pobj = GetCharacterObject(j,i);
	if(pobj==NULL)return(NULL);
	return( &(pobj->data) );
}



/*==========================================================================


	情報取得（設定）系 関数


-==========================================================================*/

DWORD CBattleTaskBase::GetWinCount(DWORD tid)
{
	return(wincount[tid]);
}

DWORD CBattleTaskBase::GetStrikerCount(DWORD tid)
{
	return(strikercount[tid]);
}

//勝利台詞を設定
void CBattleTaskBase::SetKatiSerif(DWORD tid,TCHAR *serif)
{
	g_battleresult.SetKatiSerif(tid,serif);
}

GOBJECT* CBattleTaskBase::GetActiveCharacter(DWORD tid)
{
	if(!(tid==TEAM_PLAYER1 || tid==TEAM_PLAYER2))return(NULL);
	if(active_character[tid]>=MAXNUM_TEAM){
		g_system.Log(_T("CBattleTaskBase::GetActiveCharacter , active_characterにヘンな値がはいっています"),SYSLOG_WARNING);
		return(NULL);
	}

	return( GetGObjectInfo( charobjid[tid][active_character[tid]] ) );
}

DWORD CBattleTaskBase::GetActiveCharacterID(DWORD team)
{
	if(team>=2){
		g_system.Log(_T("CBattleTaskBase::GetActiveCharacterID, 引数teamにヘンな値が指定されました"),SYSLOG_WARNING);
		return 0;
	}
	if(active_character[team]>=MAXNUM_TEAM){
		g_system.Log(_T("CBattleTaskBase::GetActiveCharacterID , active_characterにヘンな値がはいっています"),SYSLOG_WARNING);
		return 0;
	}
	return active_character[team];
}


/*==========================================================================


	描画サービス系 関数


-==========================================================================*/

//古いコードと無理矢理つじつまを合わせるためのdefine
#define sdds	g_system.GetSystemGraphicSurface()
#define srdat	g_system.GetSystemGraphicRect()
#define scdat	g_system.GetSystemGraphicCell()
#define OLDCELLDRAW(a,b,c,d,e,f,g,h,i) g_draw.CellDraw(d,f,e,c,a,b,i,0,g,h)
//古いコードと無理矢理つじつまを合わせた所にさらに強引に拡大縮小機能をねじ込むためのdefine
#define OLDCELLDRAW_NISE(a,b,c,d,e,f,g,h,i,j,k) g_draw.CellDraw(d,f,e,c,a,b,i,0,g,h,-1,j,k)

void CBattleTaskBase::DrawNumber(DWORD num,int x,int y,BOOL hits,float z, float magx, float magy)
{
	switch((num/10)%10){
	case 1:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR1,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 2:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR2,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 3:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR3,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 4:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR4,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 5:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR5,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 6:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR6,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 7:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR7,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 8:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR8,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 9:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR9,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 0:
		if(!hits)
			OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR0,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	}
	switch(num%10){
	case 1:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR1,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 2:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR2,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 3:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR3,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 4:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR4,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 5:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR5,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 6:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR6,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 7:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR7,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 8:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR8,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 9:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR9,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 0:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HR0,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	}
	if(hits){
		x+=(int)(20 * magx);
		OLDCELLDRAW_NISE(x,y,CELL_HITS_R,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);
	}

}

void CBattleTaskBase::DrawNumber2(DWORD num,int x,int y,float z)
{
	OLDCELLDRAW(x,y+5,CELL_DAMAGE_R,sdds,srdat,scdat,FALSE,FALSE,z);

	int zure1=20;
	BOOL uketa = FALSE;
	x+=40;

	switch((num/1000)%10){
	case 1:OLDCELLDRAW(x,y,CELL_NUMBER_R1,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 2:OLDCELLDRAW(x,y,CELL_NUMBER_R2,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 3:OLDCELLDRAW(x,y,CELL_NUMBER_R3,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 4:OLDCELLDRAW(x,y,CELL_NUMBER_R4,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 5:OLDCELLDRAW(x,y,CELL_NUMBER_R5,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 6:OLDCELLDRAW(x,y,CELL_NUMBER_R6,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 7:OLDCELLDRAW(x,y,CELL_NUMBER_R7,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 8:OLDCELLDRAW(x,y,CELL_NUMBER_R8,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 9:OLDCELLDRAW(x,y,CELL_NUMBER_R9,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	}
	switch((num/100)%10){
	case 1:OLDCELLDRAW(x,y,CELL_NUMBER_R1,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 2:OLDCELLDRAW(x,y,CELL_NUMBER_R2,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 3:OLDCELLDRAW(x,y,CELL_NUMBER_R3,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 4:OLDCELLDRAW(x,y,CELL_NUMBER_R4,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 5:OLDCELLDRAW(x,y,CELL_NUMBER_R5,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 6:OLDCELLDRAW(x,y,CELL_NUMBER_R6,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 7:OLDCELLDRAW(x,y,CELL_NUMBER_R7,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 8:OLDCELLDRAW(x,y,CELL_NUMBER_R8,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 9:OLDCELLDRAW(x,y,CELL_NUMBER_R9,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	default:
		if(uketa)OLDCELLDRAW(x,y,CELL_NUMBER_R0,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;
	}
	switch((num/10)%10){
	case 1:OLDCELLDRAW(x,y,CELL_NUMBER_R1,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 2:OLDCELLDRAW(x,y,CELL_NUMBER_R2,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 3:OLDCELLDRAW(x,y,CELL_NUMBER_R3,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 4:OLDCELLDRAW(x,y,CELL_NUMBER_R4,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 5:OLDCELLDRAW(x,y,CELL_NUMBER_R5,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 6:OLDCELLDRAW(x,y,CELL_NUMBER_R6,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 7:OLDCELLDRAW(x,y,CELL_NUMBER_R7,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 8:OLDCELLDRAW(x,y,CELL_NUMBER_R8,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 9:OLDCELLDRAW(x,y,CELL_NUMBER_R9,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	default:
		if(uketa)OLDCELLDRAW(x,y,CELL_NUMBER_R0,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;
	}
	switch(num%10){
	case 1:OLDCELLDRAW(x,y,CELL_NUMBER_R1,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 2:OLDCELLDRAW(x,y,CELL_NUMBER_R2,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 3:OLDCELLDRAW(x,y,CELL_NUMBER_R3,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 4:OLDCELLDRAW(x,y,CELL_NUMBER_R4,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 5:OLDCELLDRAW(x,y,CELL_NUMBER_R5,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 6:OLDCELLDRAW(x,y,CELL_NUMBER_R6,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 7:OLDCELLDRAW(x,y,CELL_NUMBER_R7,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 8:OLDCELLDRAW(x,y,CELL_NUMBER_R8,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 9:OLDCELLDRAW(x,y,CELL_NUMBER_R9,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 0:OLDCELLDRAW(x,y,CELL_NUMBER_R0,sdds,srdat,scdat,FALSE,FALSE,z);break;
	}
	x+=zure1;
	OLDCELLDRAW(x,y,CELL_PTS_R,sdds,srdat,scdat,FALSE,FALSE,z);
}

void CBattleTaskBase::DrawNumber3(double numd,int x,int y,float z)//ゲージ1用
{
	int num=(int)numd;

	switch(num%10){
	case 1:OLDCELLDRAW(x,y,CELL_PG2_NUM1,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 2:OLDCELLDRAW(x,y,CELL_PG2_NUM2,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 3:OLDCELLDRAW(x,y,CELL_PG2_NUM3,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 4:OLDCELLDRAW(x,y,CELL_PG2_NUM4,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 5:OLDCELLDRAW(x,y,CELL_PG2_NUM5,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 6:OLDCELLDRAW(x,y,CELL_PG2_NUM6,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 7:OLDCELLDRAW(x,y,CELL_PG2_NUM7,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 8:OLDCELLDRAW(x,y,CELL_PG2_NUM8,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 9:OLDCELLDRAW(x,y,CELL_PG2_NUM9,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 0:OLDCELLDRAW(x,y,CELL_PG2_NUM0,sdds,srdat,scdat,FALSE,FALSE,z);break;
	}
}

void CBattleTaskBase::DrawNumber4(double numd,int x,int y,float z)//ゲージ2用
{
	int num = (int)(numd*100.0);

	switch((num/10)%10){
	case 1:OLDCELLDRAW(x,y,CELL_PG1_NUM1,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 2:OLDCELLDRAW(x,y,CELL_PG1_NUM2,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 3:OLDCELLDRAW(x,y,CELL_PG1_NUM3,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 4:OLDCELLDRAW(x,y,CELL_PG1_NUM4,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 5:OLDCELLDRAW(x,y,CELL_PG1_NUM5,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 6:OLDCELLDRAW(x,y,CELL_PG1_NUM6,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 7:OLDCELLDRAW(x,y,CELL_PG1_NUM7,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 8:OLDCELLDRAW(x,y,CELL_PG1_NUM8,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 9:OLDCELLDRAW(x,y,CELL_PG1_NUM9,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 0:OLDCELLDRAW(x,y,CELL_PG1_NUM0,sdds,srdat,scdat,FALSE,FALSE,z);break;
	}

	x+=12;

	switch(num%10){
	case 1:OLDCELLDRAW(x,y,CELL_PG1_NUM1,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 2:OLDCELLDRAW(x,y,CELL_PG1_NUM2,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 3:OLDCELLDRAW(x,y,CELL_PG1_NUM3,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 4:OLDCELLDRAW(x,y,CELL_PG1_NUM4,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 5:OLDCELLDRAW(x,y,CELL_PG1_NUM5,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 6:OLDCELLDRAW(x,y,CELL_PG1_NUM6,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 7:OLDCELLDRAW(x,y,CELL_PG1_NUM7,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 8:OLDCELLDRAW(x,y,CELL_PG1_NUM8,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 9:OLDCELLDRAW(x,y,CELL_PG1_NUM9,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 0:OLDCELLDRAW(x,y,CELL_PG1_NUM0,sdds,srdat,scdat,FALSE,FALSE,z);break;
	}

	OLDCELLDRAW(x+12,y,CELL_PG1_PER,sdds,srdat,scdat,FALSE,FALSE,z);
}

void CBattleTaskBase::DrawNumber5(DWORD num,int x,int y,BOOL hits,float z, float magx, float magy)
{
	switch((num/10)%10){
	case 1:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL1,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 2:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL2,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 3:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL3,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 4:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL4,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 5:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL5,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 6:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL6,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 7:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL7,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 8:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL8,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 9:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL9,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 0:
		if(!hits)
			OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL0,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	}
	switch(num%10){
	case 1:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL1,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 2:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL2,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 3:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL3,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 4:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL4,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 5:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL5,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 6:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL6,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 7:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL7,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 8:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL8,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 9:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL9,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 0:OLDCELLDRAW_NISE(x,y,CELL_NUMBER_HL0,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	}
	if(hits){
		x+=(int)(20 * magx);
		OLDCELLDRAW_NISE(x,y,CELL_HITS,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);
	}

}

void CBattleTaskBase::DrawNumber6(DWORD num,int x,int y,float z)
{
	OLDCELLDRAW(x,y+5,CELL_DAMAGE,sdds,srdat,scdat,FALSE,FALSE,z);

	int zure1=20;
	BOOL uketa = FALSE;
	x+=40;

	switch((num/1000)%10){
	case 1:OLDCELLDRAW(x,y,CELL_NUMBER_L1,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 2:OLDCELLDRAW(x,y,CELL_NUMBER_L2,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 3:OLDCELLDRAW(x,y,CELL_NUMBER_L3,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 4:OLDCELLDRAW(x,y,CELL_NUMBER_L4,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 5:OLDCELLDRAW(x,y,CELL_NUMBER_L5,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 6:OLDCELLDRAW(x,y,CELL_NUMBER_L6,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 7:OLDCELLDRAW(x,y,CELL_NUMBER_L7,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 8:OLDCELLDRAW(x,y,CELL_NUMBER_L8,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 9:OLDCELLDRAW(x,y,CELL_NUMBER_L9,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	}
	switch((num/100)%10){
	case 1:OLDCELLDRAW(x,y,CELL_NUMBER_L1,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 2:OLDCELLDRAW(x,y,CELL_NUMBER_L2,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 3:OLDCELLDRAW(x,y,CELL_NUMBER_L3,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 4:OLDCELLDRAW(x,y,CELL_NUMBER_L4,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 5:OLDCELLDRAW(x,y,CELL_NUMBER_L5,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 6:OLDCELLDRAW(x,y,CELL_NUMBER_L6,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 7:OLDCELLDRAW(x,y,CELL_NUMBER_L7,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 8:OLDCELLDRAW(x,y,CELL_NUMBER_L8,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	case 9:OLDCELLDRAW(x,y,CELL_NUMBER_L9,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;uketa=TRUE;break;
	default:
		if(uketa)OLDCELLDRAW(x,y,CELL_NUMBER_L0,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;
	}
	switch((num/10)%10){
	case 1:OLDCELLDRAW(x,y,CELL_NUMBER_L1,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 2:OLDCELLDRAW(x,y,CELL_NUMBER_L2,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 3:OLDCELLDRAW(x,y,CELL_NUMBER_L3,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 4:OLDCELLDRAW(x,y,CELL_NUMBER_L4,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 5:OLDCELLDRAW(x,y,CELL_NUMBER_L5,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 6:OLDCELLDRAW(x,y,CELL_NUMBER_L6,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 7:OLDCELLDRAW(x,y,CELL_NUMBER_L7,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 8:OLDCELLDRAW(x,y,CELL_NUMBER_L8,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	case 9:OLDCELLDRAW(x,y,CELL_NUMBER_L9,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;break;
	default:
		if(uketa)OLDCELLDRAW(x,y,CELL_NUMBER_L0,sdds,srdat,scdat,FALSE,FALSE,z);x+=zure1;
	}
	switch(num%10){
	case 1:OLDCELLDRAW(x,y,CELL_NUMBER_L1,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 2:OLDCELLDRAW(x,y,CELL_NUMBER_L2,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 3:OLDCELLDRAW(x,y,CELL_NUMBER_L3,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 4:OLDCELLDRAW(x,y,CELL_NUMBER_L4,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 5:OLDCELLDRAW(x,y,CELL_NUMBER_L5,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 6:OLDCELLDRAW(x,y,CELL_NUMBER_L6,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 7:OLDCELLDRAW(x,y,CELL_NUMBER_L7,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 8:OLDCELLDRAW(x,y,CELL_NUMBER_L8,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 9:OLDCELLDRAW(x,y,CELL_NUMBER_L9,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 0:OLDCELLDRAW(x,y,CELL_NUMBER_L0,sdds,srdat,scdat,FALSE,FALSE,z);break;
	}
	x+=zure1;
	OLDCELLDRAW(x,y,CELL_PTS,sdds,srdat,scdat,FALSE,FALSE,z);
}

void CBattleTaskBase::DrawNumber7(double numd,int x,int y,float z)//ゲージ1用 TEAM2
{
	int num=(int)numd;

	switch(num%10){
	case 1:OLDCELLDRAW(x,y,CELL_PG2_NUM_R1,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 2:OLDCELLDRAW(x,y,CELL_PG2_NUM_R2,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 3:OLDCELLDRAW(x,y,CELL_PG2_NUM_R3,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 4:OLDCELLDRAW(x,y,CELL_PG2_NUM_R4,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 5:OLDCELLDRAW(x,y,CELL_PG2_NUM_R5,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 6:OLDCELLDRAW(x,y,CELL_PG2_NUM_R6,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 7:OLDCELLDRAW(x,y,CELL_PG2_NUM_R7,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 8:OLDCELLDRAW(x,y,CELL_PG2_NUM_R8,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 9:OLDCELLDRAW(x,y,CELL_PG2_NUM_R9,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 0:OLDCELLDRAW(x,y,CELL_PG2_NUM_R0,sdds,srdat,scdat,FALSE,FALSE,z);break;
	}
}

void CBattleTaskBase::DrawNumber8(double numd,int x,int y,float z)//ゲージ2用 TEAM2
{
	int num = (int)(numd*100.0);

	switch((num/10)%10){
	case 1:OLDCELLDRAW(x,y,CELL_PG1_NUM_R1,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 2:OLDCELLDRAW(x,y,CELL_PG1_NUM_R2,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 3:OLDCELLDRAW(x,y,CELL_PG1_NUM_R3,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 4:OLDCELLDRAW(x,y,CELL_PG1_NUM_R4,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 5:OLDCELLDRAW(x,y,CELL_PG1_NUM_R5,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 6:OLDCELLDRAW(x,y,CELL_PG1_NUM_R6,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 7:OLDCELLDRAW(x,y,CELL_PG1_NUM_R7,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 8:OLDCELLDRAW(x,y,CELL_PG1_NUM_R8,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 9:OLDCELLDRAW(x,y,CELL_PG1_NUM_R9,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 0:OLDCELLDRAW(x,y,CELL_PG1_NUM_R0,sdds,srdat,scdat,FALSE,FALSE,z);break;
	}

	x+=12;

	switch(num%10){
	case 1:OLDCELLDRAW(x,y,CELL_PG1_NUM_R1,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 2:OLDCELLDRAW(x,y,CELL_PG1_NUM_R2,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 3:OLDCELLDRAW(x,y,CELL_PG1_NUM_R3,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 4:OLDCELLDRAW(x,y,CELL_PG1_NUM_R4,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 5:OLDCELLDRAW(x,y,CELL_PG1_NUM_R5,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 6:OLDCELLDRAW(x,y,CELL_PG1_NUM_R6,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 7:OLDCELLDRAW(x,y,CELL_PG1_NUM_R7,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 8:OLDCELLDRAW(x,y,CELL_PG1_NUM_R8,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 9:OLDCELLDRAW(x,y,CELL_PG1_NUM_R9,sdds,srdat,scdat,FALSE,FALSE,z);break;
	case 0:OLDCELLDRAW(x,y,CELL_PG1_NUM_R0,sdds,srdat,scdat,FALSE,FALSE,z);break;
	}

	OLDCELLDRAW(x+12,y,CELL_PG1_PER_R,sdds,srdat,scdat,FALSE,FALSE,z);
}

void CBattleTaskBase::DrawNumber9(DWORD num,int x,int y,BOOL hits,float z, float magx, float magy)
{
	switch((num/10)%10){
	case 1:OLDCELLDRAW_NISE(x,y,CELL_NUMBER1,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 2:OLDCELLDRAW_NISE(x,y,CELL_NUMBER2,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 3:OLDCELLDRAW_NISE(x,y,CELL_NUMBER3,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 4:OLDCELLDRAW_NISE(x,y,CELL_NUMBER4,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 5:OLDCELLDRAW_NISE(x,y,CELL_NUMBER5,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 6:OLDCELLDRAW_NISE(x,y,CELL_NUMBER6,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 7:OLDCELLDRAW_NISE(x,y,CELL_NUMBER7,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 8:OLDCELLDRAW_NISE(x,y,CELL_NUMBER8,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 9:OLDCELLDRAW_NISE(x,y,CELL_NUMBER9,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	case 0:
		if(!hits)
			OLDCELLDRAW_NISE(x,y,CELL_NUMBER0,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);x+=(int)(20 * magx);break;
	}
	switch(num%10){
	case 1:OLDCELLDRAW_NISE(x,y,CELL_NUMBER1,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 2:OLDCELLDRAW_NISE(x,y,CELL_NUMBER2,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 3:OLDCELLDRAW_NISE(x,y,CELL_NUMBER3,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 4:OLDCELLDRAW_NISE(x,y,CELL_NUMBER4,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 5:OLDCELLDRAW_NISE(x,y,CELL_NUMBER5,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 6:OLDCELLDRAW_NISE(x,y,CELL_NUMBER6,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 7:OLDCELLDRAW_NISE(x,y,CELL_NUMBER7,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 8:OLDCELLDRAW_NISE(x,y,CELL_NUMBER8,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 9:OLDCELLDRAW_NISE(x,y,CELL_NUMBER9,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	case 0:OLDCELLDRAW_NISE(x,y,CELL_NUMBER0,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);break;
	}
	if(hits){
		x+=(int)(20 * magx);
		OLDCELLDRAW_NISE(x,y,CELL_HITS,sdds,srdat,scdat,FALSE,FALSE,z,magx,magy);
	}

}

void CBattleTaskBase::DrawGObject(GOBJECT* pdat)
{
	BOOL rev_x=FALSE;
	if(pdat->revx)rev_x=!rev_x;
	if(pdat->muki)rev_x=!rev_x;
	
	//描画の設定
	if(pdat->alphamode!=0)g_draw.SetAlphaMode(pdat->alphamode);
	if(pdat->objtype & GOBJFLG_DISPZAHYO)SetTransform(FALSE);

	g_draw.CellDraw(pdat->pmsarr,pdat->pcdat,pdat->prdat,
		pdat->cnow,(int)pdat->x,(int)pdat->y,pdat->z,
		pdat->rot,rev_x,pdat->revy,pdat->color,pdat->magx,pdat->magy);

	//描画の設定・元に戻しておく
	if(pdat->alphamode!=0)g_draw.SetAlphaMode(0);
	if(pdat->objtype & GOBJFLG_DISPZAHYO)SetTransform(TRUE);
}


const TCHAR* CBattleTaskBase::MessageID2String(DWORD id)
{
	switch(id)
	{
	case GOBJMSG_DELETE			: return _T("GOBJMSG_DELETE");
	case GOBJMSG_ACTION			: return _T("GOBJMSG_ACTION");
	case GOBJMSG_COMMAND		: return _T("GOBJMSG_COMMAND");
	case GOBJMSG_COMMANDCOM		: return _T("GOBJMSG_COMMANDCOM");
	case GOBJMSG_TOUCHA			: return _T("GOBJMSG_TOUCHA");
	case GOBJMSG_TOUCHB			: return _T("GOBJMSG_TOUCHB");
	case GOBJMSG_TOUCHC			: return _T("GOBJMSG_TOUCHC");
	case GOBJMSG_CNGAID			: return _T("GOBJMSG_CNGAID");
	//描画系メッセージ
	case GOBJMSG_DRAW			: return _T("GOBJMSG_DRAW");
	case GOBJMSG_DRAWBACK		: return _T("GOBJMSG_DRAWBACK");
	case GOBJMSG_DRAWFRONT		: return _T("GOBJMSG_DRAWFRONT");
	//登場・交代などメッセージ
	case GOBJMSG_DOTOJYO		: return _T("GOBJMSG_DOTOJYO");
	case GOBJMSG_DOTIMEOVERLOSE	: return _T("GOBJMSG_DOTIMEOVER");
	case GOBJMSG_DOYOUWIN		: return _T("GOBJMSG_DOYOUWIN");
	case GOBJMSG_TAIKI			: return _T("GOBJMSG_TAIKI");
	case GOBJMSG_KOUTAI			: return _T("GOBJMSG_KOUTAI");
	case GOBJMSG_KOUTAI2		: return _T("GOBJMSG_KOUTAI2");
	case GOBJMSG_STRIKER		: return _T("GOBJMSG_STRIKER");
	case GOBJMSG_DOYOUWIN2		: return _T("GOBJMSG_DOYOUWIN2");
	case GOBJMSG_STRIKERREADY	: return _T("GOBJMSG_STRIKERREADY");
	//座標操作系メッセージ
	case GOBJMSG_KNOCKBACK		: return _T("GOBJMSG_KNOCKBACK");
	case GOBJMSG_CLIPX			: return _T("GOBJMSG_CLIPX");
	//オブジェクト間相互作用・その他
	case GOBJMSG_CNGTARGET		: return _T("GOBJMSG_CNGTARGET");
	case GOBJMSG_SOUSAI			: return _T("GOBJMSG_SOUSAI");
	//ゲーム進行
	case GOBJMSG_CNGROUND		: return _T("GOBJMSG_CNGGROUND");
	//ネットワーク
	case GOBJMSG_SYNC			: return _T("GOBJMSG_SYNC");
	}
	static TCHAR errret[64];
	_stprintf(errret,_T("unknown-ID(0x%08X)"),id);
	return errret;
}

void CBattleTaskBase::Notify_Exception(CGObject *obj,DWORD msgid,DWORD prm)
{
	UINT team ;
	UINT index;

	switch(obj->dll_id)
	{
	case 0:
		team = 3;//system
		break;
	case 1:
		team = 0;
		index = 0;
		break;
	case 2:
		team = 0;
		index = 1;
		break;
	case 3:
		team = 0;
		index = 2;
		break;
	case 4:
		team = 1;
		index = 0;
		break;
	case 5:
		team = 1;
		index = 1;
		break;
	case 6:
		team = 1;
		index = 2;
		break;
	case 7:
		team = 2;//stage
		break;
	default:team = 3;
	}

	if(team==3)
	{
		g_system.LogErr(_T("system/unknown object , msg:%s prm:0x%08X act:0x%08X cnt:%d last_func:%s(%s)"),
			MessageID2String(msgid),
			prm,
			obj->data.aid,
			obj->data.counter,
			CExport::last_funcname,
			CExport::func_in ? _T("in") : _T("out")
			);
	}
	else if(team==2)
	{
		UINT sindex = g_battleinfo.GetStage();

		g_system.LogErr(_T("stage object %s(%s) , msg:%s prm:%08X act:0x%08X cnt:%d last_func:%s(%s)"),
			g_stagelist.GetStageName(sindex),
			(stgobjid==obj->data.id) ? _T("main"):_T("sub"),
			MessageID2String(msgid),
			prm,
			obj->data.aid,
			obj->data.counter,
			CExport::last_funcname,
			CExport::func_in ? _T("in") : _T("out")
			);
	}
	else
	{
		UINT cindex = g_battleinfo.GetCharacter(team,index);

		g_system.LogErr(_T("character object %s(%s) , msg:%s prm:0x%08X act:0x%08X cnt:%d last_func:%s(%s)"),
			g_charlist.GetCharacterName(cindex),
			(charobjid[team][index]==obj->data.id) ? _T("main"):_T("sub"),
			MessageID2String(msgid),
			prm,
			obj->data.aid,
			obj->data.counter,
			CExport::last_funcname,
			CExport::func_in ? _T("in") : _T("out")
			);
	}
}


const TCHAR* CBattleTaskBase::GetBattleStateString()
{
	switch(bf_state)
	{
	case BFSTATE_WAITFORENDPOSE	:return _T("BFSTATE_WAITFORENDPOSE");
	case BFSTATE_ROUNDCALL		:return _T("BFSTATE_ROUNDCALL");
	case BFSTATE_FIGHTING		:return _T("BFSTATE_FIGHTING");
	case BFSTATE_FINISHED		:return _T("BFSTATE_FINISHED");
	case BFSTATE_WAITFORENDWIN	:return _T("BFSTATE_WAITFORENDWIN");
	case BFSTATE_DOUBLEKO		:return _T("BFSTATE_DOUBLEKO");
	case BFSTATE_TIMEOVER		:return _T("BFSTATE_TIMEOVER");
	}
	return _T("BFSTATE_UNKNOWN");
}


