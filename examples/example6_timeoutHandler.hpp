

/** Time out handler.
  * Used to stop the current operation after too much time, or if the user
  * requested cancellation.
  */
class timeoutHandler : public vmime::net::timeoutHandler
{
public:

	bool isTimeOut()
	{
		// This is a cancellation point: return true if you want to cancel
		// the current operation. If you return true, handleTimeOut() will
		// be called just after this, and before actually cancelling the
		// operation
		return false;
	}

	void resetTimeOut()
	{
		// Called at the beginning of an operation (eg. connecting,
		// a read() or a write() on a socket...)
	}

	bool handleTimeOut()
	{
		// If isTimeOut() returned true, this function will be called. This
		// allows you to interact with the user, ie. display a prompt to
		// know whether he wants to cancel the operation.

		// If you return true here, the operation will be actually cancelled.
		// If not, the time out is reset and the operation continues.
		return true;
	}
};


class timeoutHandlerFactory : public vmime::net::timeoutHandlerFactory
{
public:

	vmime::shared_ptr <vmime::net::timeoutHandler> create()
	{
		return vmime::make_shared <timeoutHandler>();
	}
};

