

/*============================================================================

	ファイルへのログ保存

==============================================================================*/
#pragma once

/*!
*	@brief テキストファイルへのログ保存クラス
*	@ingroup global
*
*	system/log/ にテキストを保存します。
*	一定量のバッファを持ち、それがいっぱいにならないと出力しません。
*	なので、ログがファイルへ吐き出される前（メモリ上にあるうち）に
*	いきなり落ちるとそのログは失われます。
*	重要なログを出力する場合はFlush()を呼ぶとバッファーされているログが
*	即座にファイルへ書き出されます。
*/
class CLog2File
{
public:
	/*! コンストラクタで取得された日付と時間で出力ファイル名が決定されます。 */
	CLog2File();
	~CLog2File();

	/*!
		@brief ログを追加する。
	一定量ログがたまると自動的にバッファからファイルへ書き出す */
	void AddLog(char *str);

	/*!
		@brief バッファに保持しているログをファイルへ書き出す。
		通常はAddLog()かデストラクタで行われるので明示的にコールする必要はない
	*/
	void Flush();

private:
	char *buf;
	char *p;
	char *filename;
};
