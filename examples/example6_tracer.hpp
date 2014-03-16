
/** Tracer used to demonstrate logging communication between client and server.
  */

class myTracer : public vmime::net::tracer
{
public:

	myTracer(vmime::shared_ptr <std::ostringstream> stream,
	         vmime::shared_ptr <vmime::net::service> serv, const int connectionId)
		: m_stream(stream), m_service(serv), m_connectionId(connectionId)
	{
	}

	void traceSend(const vmime::string& line)
	{
		*m_stream << "[" << m_service->getProtocolName() << ":" << m_connectionId
		          << "] C: " << line << std::endl;
	}

	void traceReceive(const vmime::string& line)
	{
		*m_stream << "[" << m_service->getProtocolName() << ":" << m_connectionId
		          << "] S: " << line << std::endl;
	}

private:

	vmime::shared_ptr <std::ostringstream> m_stream;
	vmime::shared_ptr <vmime::net::service> m_service;
	const int m_connectionId;
};

class myTracerFactory : public vmime::net::tracerFactory
{
public:

	myTracerFactory(vmime::shared_ptr <std::ostringstream> stream)
		: m_stream(stream)
	{
	}

	vmime::shared_ptr <vmime::net::tracer> create
		(vmime::shared_ptr <vmime::net::service> serv, const int connectionId)
	{
		return vmime::make_shared <myTracer>(m_stream, serv, connectionId);
	}

private:

	vmime::shared_ptr <std::ostringstream> m_stream;
};

