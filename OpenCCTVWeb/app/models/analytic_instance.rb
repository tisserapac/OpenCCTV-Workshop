class AnalyticInstance < ActiveRecord::Base
  belongs_to :analytic
  has_many :analytic_instance_streams, dependent: :destroy
  has_many :streams, through: :analytic_instance_streams

  validates :analytic_id, presence: true
  validates :name, presence: true

  def exec_analytic_event(operation_name)
    msg_details = nil
    message = nil

    if(operation_name == 'Analytic Start')
      message = get_analytic_start_request
    elsif(operation_name == 'Analytic Stop')
      message = get_analytic_stop_request
    else
      msg_details[:type] = 'Error'
      msg_details[:content] = "Invalid operation #{operation_name}"
      return msg_details
    end

    if(message.nil?)
      msg_details[:type] = 'Error'
      msg_details[:content] = "Error in #{operation_name} : Failed to build the XML message"
      return msg_details
    end

    @server = OpenCctvServer.last
    if(@server.nil?)
      msg_details[:type] = 'Error'
      msg_details[:content] = "Error in #{operation_name} : Currently no OpenCCTV Server is registered with the system."
      return msg_details
    end

    reply = 'Error'

    context = ZMQ::Context.new(1)
    requester = nil

    if context
      #Create a request type socket
      requester = context.socket(ZMQ::REQ)

      #Set the socket options
      requester.setsockopt(ZMQ::SNDTIMEO, 10000) # A 10 second timeout is set for send
      requester.setsockopt(ZMQ::RCVTIMEO, 10000) # A 10 second timeout is set for receive
      requester.setsockopt(ZMQ::LINGER,0)

      #Connects to the socket
      rc = requester.connect("tcp://#{@server.host}:#{@server.port}")

      #Send a request
      rc = requester.send_string(message) unless zmq_error_check(rc)

      #Receive reply
      rc = requester.recv_string(reply) unless zmq_error_check(rc)
    end

    #Close the socket and terminate the ZMQ context
    requester.close unless requester.nil?
    context.terminate

    if(reply.eql? 'Error')
      msg_details[:type] = 'Error'
      msg_details[:content] = "Error in #{operation_name} : Failed to receive a reply from the OpenCCTV Server"
      return msg_details
    end

    msg_details = parse_reply(reply)
    return msg_details
  end

  private

  def parse_reply (xml_string)
    #Example reply messages :
    #successfull : <?xml version="1.0" encoding="utf-8"?><opencctvmsg><type>AnalyticStartReply</type><content>Analytic instance 4 started</content></opencctvmsg>
    #failed : <?xml version="1.0" encoding="utf-8"?> <opencctvmsg><type>Error</type><content>Unknown message type received</content><serverstatus>Unknown</serverstatus><serverpid>0</serverpid></opencctvmsg>

    type = nil
    content = nil

    msg_details = {}


    if(!xml_string.nil? && xml_string.start_with?("<"))
      doc = Nokogiri::XML(xml_string)
      type = doc.xpath('//opencctvmsg//type')[0].content.to_s.strip
      content = doc.xpath('//opencctvmsg//content')[0].content.to_s.strip

      msg_details[:type] = type
      msg_details[:content] = content
    end

    return msg_details

  end

  def get_analytic_start_request
    #<?xml version=1.0 encoding=utf-8?><opencctvmsg><type>StartAnalyticInstance</type><analyticinstanceid>4</analyticinstanceid><analyticplugindir>MockAnalytic</analyticplugindir><inputstreams><inputstream><streamid>2</streamid><name>default</name></inputstream><inputstream><streamid>3</streamid><name>low res</name></inputstream></inputstreams></opencctvmsg>
    message = nil

    builder = Nokogiri::XML::Builder.new do |xml|
      xml.opencctvmsg {
        xml.type "StartAnalyticInstance"
        xml.analyticinstanceid self.id
        xml.analyticplugindir self.analytic.filename
        xml.inputstreams{
          self.analytic_instance_streams.each do |instance_stream|
            xml.inputstream{
              xml.streamid instance_stream.stream_id
              xml.name instance_stream.analytic_input_stream.name
            }
          end
        }
      }
    end

    message = builder.to_xml
    return message
  end

  def get_analytic_stop_request
    #<?xml version="1.0" encoding="utf-8"?><opencctvmsg><type>StopAnalyticInstance</type><analyticinstanceid>1</analyticinstanceid><inputstreams><streamid>1</streamid><streamid>2</streamid></inputstreams></opencctvmsg>
    message = nil

    builder = Nokogiri::XML::Builder.new do |xml|
      xml.opencctvmsg {
        xml.type "StopAnalyticInstance"
        xml.analyticinstanceid self.id
        xml.inputstreams{
          self.analytic_instance_streams.each do |instance_stream|
            xml.streamid instance_stream.stream_id
          end
        }
      }
    end

    message = builder.to_xml
    return message
  end

  def zmq_error_check(rc)
    if ZMQ::Util.resultcode_ok?(rc)
      false
    else
      STDERR.puts "Operation failed, errno [#{ZMQ::Util.errno}] description [#{ZMQ::Util.error_string}]"
      true
    end
  end

end
