/*----------------------------------------------------------------------

	ポーズ画面

------------------------------------------------------------------------*/

#include "task.h"
#include "dx_draw.h"

/*!
*	@brief 試合をポーズしたときの操作・描画を行う
*	@ingroup Battle
*/
class CTBattlePause : public CTaskBase
{
public:
	void Initialize();						//ExecuteまたはDrawがコールされる前に1度だけコールされる
	void Terminate();						//タスクのリストから外されるときにコールされる（その直後、deleteされる）
	BOOL Execute(DWORD time);				//毎フレームコールされる
	void Draw();							//描画時にコールされる
	int GetDrawPriority(){return 500;}		//描画プライオリティ。低いほど手前に（後に）描画。マイナスならば表示しない

	void Kill()	{m_kill_flag=TRUE;}
	BOOL GetKillFlag() {return m_kill_flag;}
	void ChangeInst();

protected:
	LPDIRECT3DTEXTURE8 tex_fb;				//!< フロントバッファをコピーしたテクスチャ
	LPDIRECT3DTEXTURE8 tex_pause;			//!< "Pause Press F7 Key" 表示用テクスチャ
	MYSURFACE* ms_inst;						//!< インストbmp

	UINT m_counter;
	UINT m_face_counter;
	UINT m_face_idx[2];
	UINT m_face_teamid;
	BOOL m_kill_flag;
	BOOL m_inst_on;
	int m_shiftY;
	DWORD m_selected;
};

//selectedgamemodeの項目
#define CONTINUE_BATTLE			0
#define OPEN_INST				1
#define RETURN_TO_TITLE			2//!< これを項目の最後とみなしている
