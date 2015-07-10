#pragma once

using namespace System::Threading;

namespace openalprnet
{
	private ref class Lock {
		Object^ m_pObject;
	public:
		Lock(Object ^ pObject) : m_pObject(pObject) {
			Monitor::Enter(m_pObject);
		}
		~Lock() {
			Monitor::Exit(m_pObject);
		}
	};
}

