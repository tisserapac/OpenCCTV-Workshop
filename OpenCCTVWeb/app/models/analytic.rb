class Analytic < ActiveRecord::Base
  has_many :analytic_input_streams, dependent: :destroy
  has_many :analytic_instances, dependent: :destroy

  validates :name, presence: true
  validates_presence_of :filename, message: 'No analytic plugin archive selected to upload.'

  def validate_plugin_archive

    validation_result = {}
    verified = false
    html_content = nil
    input_streams = Array.new

    if(!self.filename.empty?)
      cmd = "#{Rails.root}/app/assets/programs/PluginArchiveValidator/Release/PluginArchiveValidator " +
            "#{Rails.root}/app/uploads/analytics " + "#{self.filename}.zip " + "#{self.filename} " + 'analytic'
      stdin, stdout, stderr = Open3.popen3(cmd)
      output = stdout.readline
      if(!output.nil? && (output.start_with?("<")))
        xml = Nokogiri::XML(output)

        #Verification results
        if(xml.xpath('//pluginvalidatorreply/verified')[0].content == '1')
          verified = true
        end

        #HTML content
        html_content = xml.xpath('//pluginvalidatorreply/html')[0].inner_html

        #Input stream details
        xml.xpath('//pluginvalidatorreply/analytic/inputstreams/input').each do |in_stream|
          stream = AnalyticInputStream.new
          stream.name = in_stream.xpath("name")[0].content
          stream.description = in_stream.xpath("description")[0].content
          input_streams.push(stream);
        end
      end
    end

    validation_result[:verified] = verified
    validation_result[:html_content] = html_content
    validation_result[:input_streams] = input_streams

    return validation_result
  end

  def exec_stop_analytics
    msg_details = {}
    message = nil

    message = get_analytics_stop_request

    if(message.nil? || message.blank?)
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

  def get_analytics_stop_request
    #<?xml version="1.0" encoding="utf-8"?><opencctvmsg><type>StopAnalyticInstances</type><analyticid>1</analyticid><analyticinstances><analyticinstanceid>1</analyticinstanceid></analyticinstances></opencctvmsg>
    message = ''

    builder = Nokogiri::XML::Builder.new do |xml|
      xml.opencctvmsg {
        xml.type "StopAnalyticInstances"
        xml.analyticid self.id
        xml.analyticinstances{
          self.analytic_instances.each do |analytic_instance|
            xml.analyticinstanceid analytic_instance.id
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
