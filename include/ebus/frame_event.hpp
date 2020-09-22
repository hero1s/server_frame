#pragma once

#include "event_bus.hpp"
#include <string>

//����¼�
namespace ebus {
	class NewDayEvent : public Event {
	public:
		NewDayEvent(Object& sender, bool bNewWeek, bool bNewMonth)
				:
				Event(sender),
				_newWeek(bNewWeek),
				_newMonth(bNewMonth)
		{
		}

		virtual ~NewDayEvent() { }

		bool isNewWeek() { return _newWeek; }

		bool isNewMonth() { return _newMonth; }

	private:
		bool _newWeek;
		bool _newMonth;

	};
};

