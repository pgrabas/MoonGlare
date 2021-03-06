/*
 * DataClasses.h
 *
 *  Created on: 20-10-2013
 *      Author: Paweu
 */
#pragma once

#define xmlAttr_Class				"Class"

namespace MoonGlare::DataClasses {

	template<class T>
	struct ResourceFinalizer {
		void operator()(T *t) {
			if (!t->Finalize()) {
				AddLog(Error, "Unable to finalize resource " << t->GetName());
			}
			delete t;
		}
	};

	namespace Fonts {
		class iFont;
		using FontPtr = std::shared_ptr < iFont > ;

		enum class FontStyleFlags {
			Bold		= 0x01,
			Italic		= 0x02,
			Underline	= 0x04,
			StrikeOut	= 0x08,
		};

		struct Descriptor {
			float Size = 0.0f;
			math::fvec3 Color = math::fvec3(1.0f);

			//DefineFlag(m_Flags, (unsigned)FontStyleFlags::Bold, Bold);
			//DefineFlag(m_Flags, (unsigned)FontStyleFlags::Italic, Italic);
			//DefineFlag(m_Flags, (unsigned)FontStyleFlags::Underline, Underline);
			//DefineFlag(m_Flags, (unsigned)FontStyleFlags::StrikeOut, StrikeOut);
		protected:
			//unsigned m_Flags = 0;
		};
	}
	using Fonts::FontPtr;

	class DataClass;
} //namespace MoonGlare::DataClasses
