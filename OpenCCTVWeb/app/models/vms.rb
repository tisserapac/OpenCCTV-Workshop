class Vms < ActiveRecord::Base
  belongs_to :vms_connector
  has_many :cameras, dependent: :destroy
  validates :name, presence: true
  validates :server_ip, presence: true
  validates :server_port, presence: true
  validates :vms_connector_id, presence: true
  validates :vms_type, presence: true
  validates :camera_url, presence: true

  ## Public methods start

  # validates all VMS types and update verified column of table if VMS is valid
  def validate_connection
    valid = false
    if(self.vms_type == 'milestone')
      valid, xml_response = milestone_validate_vms
    elsif(self.vms_type == 'zoneminder')
      valid = zoneminder_validate_vms
    elsif(self.vms_type == 'hikvision')
      valid = hikvision_validate_device
    else # anything else
      valid = true
    end
    if(valid)
      self.update(:verified => true)
    end
    return valid
  end

  # adds cameras and streams to the vms
  def add_cameras
    if(self.vms_type == 'milestone')
      is_vms_valid, xml_response = milestone_validate_vms
      if(is_vms_valid)
        return milestone_add_cameras(xml_response)
      end
    elsif(self.vms_type == 'zoneminder')
      url = create_zoneminder_url(self.server_ip, self.server_port)
      cookie = zoneminder_login(url, self.username, self.password)
      if(!cookie.nil?)
        zoneminder_add_monitors(url, cookie)
        zoneminder_add_streams(url, cookie, self.cameras)
      end
    elsif(self.vms_type == 'hikvision')
      hikvision_add_monitors
    else
    end
  end

  def set_verification(verification)
    self.update(:verified => verification)
    self.cameras.each do |camera|
      camera.set_verification(verification)
    end
  end

  # Sample output of the program DirectCameraInitializer
  # DirectCameraInitializer is OpenCV based
  # <camerainitializerreply><verified>1</verified><message>Camera verification successful</message><width>640</width><height>480</height></camerainitializerreply>
  def init_direct_ip_camera
    filename = "#{Rails.root}/app/assets/images/#{self.id}.jpeg"
    output = nil

    cmd = "#{Rails.root}/app/assets/programs/DirectCameraInitializer/Release/DirectCameraInitializer "+
          "\"#{self.camera_url}\" " + "#{filename}"

    Open3.popen3(cmd) do |stdin, stdout, stderr, wait_thr|
      exit_status = wait_thr.value
      if exit_status.success?
        output = stdout.readline
      end
    end

    #stdin, stdout, stderr = Open3.popen3(cmd)
    #output = stdout.readline

    camera_verified = false
    width = 0
    height = 0

    if(!output.nil? && (output.start_with?("<")))
      xml = Nokogiri::XML(output)
      if(xml.xpath('//camerainitializerreply/verified')[0].content == '1')
        camera_verified = true
        width = xml.xpath('//camerainitializerreply/width')[0].content
        height = xml.xpath('//camerainitializerreply/height')[0].content
      end
    end

    #self.verified = camera_verified

    self.update(:verified => camera_verified)

    # If camera_verified is true then add the camera and the default stream
    if(camera_verified)
      camera = self.cameras.first

      if camera.nil?
        camera = Camera.new
        camera.name = self.name
        camera.camera_id = self.name
        camera.description = "Direct Camera - #{self.description}"
        camera.verified = camera_verified
        self.cameras.push(camera)
      else
        camera.update(:name => self.name, :camera_id => self.name, :description => self.description, :verified => camera_verified )
      end

      default_stream = camera.streams.first
      if default_stream.nil?
        default_stream = Stream.new(:name => "Default stream - #{self.name}", :width => width, :height => height, :keep_aspect_ratio => true, :allow_upsizing => false, :compression_rate => 100, :description => "Default stream from a direct camera", :verified => camera_verified)
        camera.streams.push(default_stream)
      else
        default_stream.update(:name => "Default stream - #{self.name}")
      end

      # Default image for the camera and the stream
      if File.exist?(filename)
        cam_filename = "#{Rails.root}/app/assets/images/#{self.id}_#{camera.id}.jpeg"
        stream_filename = "#{Rails.root}/app/assets/images/#{self.id}_#{camera.id}_#{default_stream.id}.jpeg"

        in_file = open(filename, 'rb')
        indata = in_file.read

        out_file = open(cam_filename, 'wb')
        out_file.write(indata)
        out_file.close

        out_file = open(stream_filename, 'wb')
        out_file.write(indata)
        out_file.close

        in_file.close

        File.delete(filename)
      end
    end

    return camera_verified
  end

  ## Public methods end

  # Private methods start

  private
  # validates a Milestone VMS
  # returns (true, systeminfo_xml_string) if valid or (false, error_msg) if invalid
  def milestone_validate_vms
    response = connect_to_milestone_enterprise(self.server_ip, self.server_port, self.username, self.password)
    if response.nil? || response.code == '404' # if a enterprise server cannot be found
      response = connect_to_milestone_corporate(self.server_ip, self.server_port, self.username, self.password)
    end
    error_msg = nil
    if !response.nil?
      if response.code == '200' # HTTP OK
        return true, response.body
      elsif response.code == '404' # HTTP not found
        error_msg = 'Failed to connect to Milestone XProtect server. Invalid Server IP or Port.'
      elsif response.code == '401' # HTTP unauthorized
        error_msg = 'Failed to connect to Milestone XProtect server. Invalid Username or Password.'
      else
        error_msg = "We could not connect to the Milestone VMS. HTTP error: #{response.message}"
      end
    else
      error_msg = 'Failed to connect to Milestone XProtect server.'
    end
    if(!error_msg.nil?)
      raise error_msg
    end
    return false, error_msg
  end

  # validates a ZoneMinder VMS
  def zoneminder_validate_vms
    url = create_zoneminder_url(self.server_ip, self.server_port)
    cookie = zoneminder_login(url, self.username, self.password)
    error_msg = nil
    if(!cookie.nil?)
      monitors = zoneminder_get_monitors(url, cookie)
      if(!monitors.nil?)
        return true
      else
        error_msg = 'Failed to retrieve Monitor information from the ZoneMinder server.'
      end
    else
      error_msg = 'Failed to connect to ZoneMinder server.'
    end
    if(!error_msg.nil?)
      raise error_msg
    end
    return false
  end

  # validates HikVision devices (DVRs, NVRs, Cameras)
  def hikvision_validate_device
    response = connect_to_hikvision_device(self.server_ip, self.server_port, self.username, self.password)
    if(validate_http_response(response))
      return true
    else
      raise 'Failed to connect to HikVision device.'
    end
  end

  # extract milestone camera info from milestone system info xml
  def milestone_add_cameras(xml_string)
    doc = Nokogiri::XML(xml_string)
    doc.xpath("//camera").each do |cam_ele|
      camera = Camera.new
      camera.name = cam_ele.attribute("cameraid").to_s
      camera.camera_id = cam_ele.xpath("guid")[0].content
      self.cameras.push(camera)
      is_default_stream_valid, default_width, default_height = camera.milestone_grab_default_stream_frame
      if(is_default_stream_valid)
        default_stream = Stream.new(:name => "Default - #{camera.name}", :width => default_width, :height => default_height, :keep_aspect_ratio => false, :allow_upsizing => false, :compression_rate => 90, :description => "Original Stream from Camera")
        camera.update(:verified => true)
        camera.streams.push(default_stream)
        if(default_stream.milestone_grab_stream_frame)
          default_stream.update(:verified => true)
        end
      end
    end
    return cameras
  end

  # getMonitors returns a HashMap<monitorName, monitorWatchPath>,
  # nil if an error occurred or empty HashMap if no monitors
  def zoneminder_get_monitors(url, cookie)
    monitors = nil # return val
    response = Unirest.get(url, headers:{"Cookie" => cookie.join(" ")})
    if response.code == 200
      monitors = Hash.new # a hashmap
      page = Nokogiri::HTML(response.body)
      page.css('tbody//tr//td[class="colName"]').each do |monitorInfo|
        monitorName = monitorInfo.inner_text
        monitorWatchPath = monitorInfo.children[0].attribute("href")
        monitors[monitorName] = monitorWatchPath
      end
    else
      raise 'Failed to connect to the ZoneMinder server.'
    end
    return monitors
  end

  # zoneminder get monitors returns a HashMap<monitorName, monitorWatchPath>,
  # nil if an error occurred or empty HashMap if no monitors
  def zoneminder_add_monitors(url, cookie)
    response = Unirest.get(url, headers:{"Cookie" => cookie.join(" ")})
    if(response.code == 200)
      page = Nokogiri::HTML(response.body)
      page.css('tbody//tr//td[class="colName"]').each do |monitorInfo|
        camera = Camera.new
        camera.name = monitorInfo.inner_text
        params = monitorInfo.children[0].attribute("href").inner_text
        camera.camera_id = params.split('=')[-1] # get string after last '='
        if(!camera.name.nil? && !camera.camera_id.nil?)
          camera.verified = true
        end
        self.cameras.push(camera)
      end
    elsif(response.code == 404)
      raise 'Failed to connect to the ZoneMinder server. Invalid Server URL or Port.'
    elsif(response.code == 401)
      raise 'Failed to connect to the ZoneMinder server. Invalid Username or Password.'
    else
      raise 'Failed to connect to the ZoneMinder server.'
    end
  end

  def hikvision_add_monitors
    response = connect_to_hikvision_device(self.server_ip, self.server_port, self.username, self.password)
    doc = Nokogiri::XML(response.body) # response is an xml
    doc.remove_namespaces!
    doc.xpath('//StreamingChannelList/StreamingChannel').each do |channel|
      if(channel.xpath('enabled')[0].content.to_s == 'true' &&
          channel.xpath('//ControlProtocol/streamingTransport')[0].inner_text == 'RTSP' &&
          channel.xpath('//Video/enabled')[0].inner_text == 'true')
        camera = Camera.new
        camera.camera_id = channel.xpath('id')[0].inner_text
        camera.description = channel.xpath('channelName')[0].inner_text
        camera.name = camera.camera_id
        if(!camera.name.nil? && !camera.camera_id.nil?)
          camera.verified = true
        end
        self.cameras.push(camera)
        camera.hikvision_grab_default_frame
        stream = Stream.new
        stream.name = "Default - #{camera.name}"
        stream.width = channel.xpath('//Video/videoResolutionWidth')[0].inner_text.to_i
        stream.height = channel.xpath('//Video/videoResolutionHeight')[0].inner_text.to_i
        stream.compression_rate = channel.xpath('//Video/fixedQuality')[0].inner_text.to_i
        stream.keep_aspect_ratio = false
        stream.allow_upsizing = false
        stream.camera = camera
        stream.save
        stream.update(:verified => true)
      end
    end
  end

  def connect_to_milestone_enterprise(server_name, server_port, username, password)
    url = "http://#{server_name}/systeminfo.xml"
    return connect_with_basic_auth(url, server_port.to_i, username, password)
  end

  def connect_to_milestone_corporate(server_name, server_port, username, password)
    url = "http://#{server_name}/rcserver/systeminfo.xml"
    return connect_with_basic_auth(url, server_port.to_i, username, password)
  end

  # HTTP with basic auth. returns HTTP respose
  def connect_with_basic_auth(url, port_number, username, password)
    uri = URI.parse(url)
    http = Net::HTTP.new(uri.host, port_number)
    http.open_timeout = 5
    http.read_timeout = 5
    request = Net::HTTP::Get.new(uri.request_uri)
    request.basic_auth(username, password)
    begin
      return http.request(request)
    rescue
      raise 'Failed to connect to the Server using provided IP and Port.'
    end
  end

  # validate HTTP response
  def validate_http_response(response)
    if(!response.nil?)
      if(response.code == '404') # host could not found
        raise 'Invalid Server IP or Server Port.'
      elsif(response.code == '200')
        return true
      elsif(response.code == '401')
        raise 'Invalid Username or Password.'
      else
        raise "Could not connect to the Server. HTTP error: #{response.message}."
      end
    end
    return false
  end

  # zoneminder login returns cookie (as an Array) after successful auth or nil if login failed
  def zoneminder_login(url, username, password)
    cookie = nil # return val
    response = Unirest.post(url, parameters:{ :action => "login", :view => "postlogin", :username => username, :password => password })
    error_msg = nil
    if response.code == 200
      if response.body.include? "username" and response.body.include? "password" #again login form
      elsif(response.body.include? 'Logging In') #got auth correct
        cookie = response.headers[:set_cookie]
      else #unknown response
        error_msg = 'Failed to connect to the ZoneMinder server. Unknown response from ZoneMinder server.'
      end
    else
      error_msg = 'Failed to connect to the ZoneMinder server.'
    end
    if(!error_msg.nil?)
      raise error_msg
    end
    return cookie
  end

=begin
  # zoneminder get stream info of each monitor
  # returns a HashMap<monitorName, Array[streamUrl, streamWidth, streamHeight]>
  # returns nil if an error occurred or empty HashMap if no streams
  def zoneminder_get_streams(url, cookie, monitors)
    streams = nil # return val
    if !monitors.empty?
      streams = Hash.new # a hashmap
      monitors.each do |monitorName, monitorWatchPath|
        monitorWatchUrl = "#{url}#{monitorWatchPath}"
        response = Unirest.post(monitorWatchUrl, headers:{"Cookie" => cookie.join(" ")})
        if response.code == 200
          page = Nokogiri::HTML(response.body)
          streamInfo = page.css('div#imageFeed//img#liveStream')[0]
          if(!streamInfo.nil?)
            streamUrl = streamInfo.attribute("src")
            streamWidth = streamInfo.attribute("width")
            streamHeight = streamInfo.attribute("height")
            streams[monitorName] = [streamUrl, streamWidth, streamHeight]
          end
        else
          puts "Error [getStreams]: Cannot connect to host."
        end
      end
    end
    return streams
  end
=end

  def zoneminder_add_streams(url, cookie, monitors)
    monitors.each do |camera|
      monitorWatchUrl = "#{url}?view=watch&mid=#{camera.camera_id}"
      response = Unirest.post(monitorWatchUrl, headers:{"Cookie" => cookie.join(" ")})
      if(response.code == 200)
        page = Nokogiri::HTML(response.body)
        streamInfo = page.css('div#imageFeed//img#liveStream')[0]
        if(!streamInfo.nil?)
          stream = Stream.new
          stream.name = "Default - #{camera.name}"
          streamUrl = streamInfo.attribute("src").inner_text
          width = streamInfo.attribute("width").inner_text
          height = streamInfo.attribute("height").inner_text
          if(!streamUrl.nil?)
            stream.width = width.to_i
            stream.height = height.to_i
            stream.compression_rate = 100
            stream.keep_aspect_ratio = false
            stream.allow_upsizing = false
            stream.camera = camera
            stream.save
            done, new_width, new_height = stream.zoneminder_grab_frame(streamUrl)
            if(done)
              stream.update(:verified => true)
              stream.update(:width => new_width.to_i)
              stream.update(:height => new_height.to_i)
            else
              stream.update(:verified => false)
            end
          end
        else
          raise 'Failed to retrieve Monitor Streams from the Zoneminder server.'
        end
      else
        raise 'Failed to connect to the ZoneMinder server.'
      end
    end
  end

=begin
  def zoneminder_add_stream(url, cookie, camera)
    monitorWatchUrl = "#{url}#{camera.camera_id}"
    response = Unirest.post(monitorWatchUrl, headers:{"Cookie" => cookie.join(" ")})
    if(response.code == 200)
      page = Nokogiri::HTML(response.body)
      streamInfo = page.css('div#imageFeed//img#liveStream')[0]
      if(!streamInfo.nil?)
        stream = Stream.new
        stream.name = "Default - #{camera.name}"
        streamUrl = streamInfo.attribute("src")
        stream.description = streamUrl
        stream.width = streamInfo.attribute("width").to_s.to_i
        stream.height = streamInfo.attribute("height").to_s.to_i
        camera.streams.push(stream)
      end
    else
      raise 'Failed to connect to ZoneMinder server.'
    end
  end
=end

  def create_zoneminder_url(ip, port)
    if(port == 80)
      return "http://#{ip}/index.php"
    end
    return "http://#{ip}:#{port}/index.php"
  end

  def connect_to_hikvision_device(server_name, server_port, username, password)
    url = "http://#{server_name}/ISAPI/Streaming/channels"
    return connect_with_basic_auth(url, server_port.to_i, username, password)
  end

  # Private methods end

end