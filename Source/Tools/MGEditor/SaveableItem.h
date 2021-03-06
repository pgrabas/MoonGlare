/*
  * Generated by cppsrc.sh
  * On 2016-07-21 21:39:11,81
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef SaveableItem_H
#define SaveableItem_H

namespace MoonGlare {
namespace Editor {

class SaveableItem;
using WeakSaveableItem = std::weak_ptr<SaveableItem>;

class SaveableItem : public std::enable_shared_from_this<SaveableItem> {
public:
 	SaveableItem();
 	virtual ~SaveableItem();

	virtual void PeriodicSaveNotification();

	bool IsChanged() const { return m_Changed; }

	void AddChild(WeakSaveableItem child);
	virtual void ChildModiffied(WeakSaveableItem child);
protected:
	virtual bool DoPeriodicSave();
	virtual void SetChanged();
private: 
	bool m_Changed = false;
	bool m_ChildChanged = false;
	WeakSaveableItem m_Parent;
	std::list<WeakSaveableItem> m_Children;
};

class SaveableItemParent : public QObject, public SaveableItem {
	Q_OBJECT;
public:
	SaveableItemParent();

	virtual void ChildModiffied(WeakSaveableItem child) override;
protected:
	virtual void SetChanged() override;
	std::unique_ptr<QTimer> m_Timer;
public slots:
	void SaveNow();
private slots:
	void SaveTimeout();
};

} //namespace Editor 
} //namespace MoonGlare 

#endif
