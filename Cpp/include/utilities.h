#pragma once

#include <set>
#include <iostream>


template<typename SubjectType>
class Observer
{
protected:
	SubjectType* subject_;

public:
	Observer(SubjectType* subject) :
		subject_(subject)
	{
		subject_->attach(this);
	}

	virtual ~Observer()
	{
		subject_->detach(this);
	}

	// Todo : detach
	virtual void update()
	{
		std::cout << "notify()" << std::endl;
	}
};

template<typename SubjectType>
class Subject
{
	typedef Observer<SubjectType> SpecificObserver;
	std::set<SpecificObserver*> observers;

public:
	void attach(SpecificObserver* observer)
	{
		observers.insert(observer);
	}

	void detach(SpecificObserver* observer)
	{
		observers.erase(observer);
	}

	void notify()
	{
		for(std::set<SpecificObserver*>::iterator observer_it = observers.begin(); 
			observer_it != observers.end(); 
			++observer_it)
		{
			(*observer_it)->update();
		}
	}
};
