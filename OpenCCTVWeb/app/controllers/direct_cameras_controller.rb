class DirectCamerasController < ApplicationController
  before_action :set_vms, only: [:show, :edit, :update, :destroy]
  before_action :set_vms_connector, only: [:show, :edit, :update]
  respond_to :html

  def index
    @vmses = Vms.joins(:vms_connector).where('vms_connectors.connector_type = "direct_ip_camera"')
  end

  def show
    @camera = nil
    @strema = nil

    @camera = @vms.cameras.first
    unless @camera.nil?
      @stream = @camera.streams.first
    end

    respond_with @vms
  end

  def new
    @vms = Vms.new
  end

  def edit
  end

  def create
    @vms = Vms.new(vms_params)
    @vms.server_ip = 'NA'
    @vms.server_port = 0
    @vms.username = 'NA'
    @vms.password = 'NA'
    # Carry out actual validation of the direct camera
    # @vms.verified = true
    #TODO Assign a suitable VMS type
    @vms.vms_type = 'NA'

    if @vms.save
      if(@vms.init_direct_ip_camera)
        flash[:notice] = 'Camera details was successfully saved.'
      else
        flash[:error] = 'Validating and initializing the camera failed'
      end
      redirect_to direct_cameras_show_url @vms
      #redirect_to direct_cameras_show_url @vms, notice:'Camera details was successfully saved.'
    else
      render :new
    end
  end

  def update
    @vms.update(vms_params)

    if !@vms.errors.any?
      # Validate, update the camera and the stream details
      if(@vms.init_direct_ip_camera)
        flash[:notice] = 'Camera details was successfully updated.'
      else
        flash[:error] = 'Validating and initializing the camera with updated details failed'
      end
      redirect_to direct_cameras_url
    else
      render :edit
    end
  end

  def destroy
    is_instance_streams = false

    @vms.cameras.each do |camera|
      camera.streams.each do |stream|
        if(!AnalyticInstanceStream.find_by_stream_id(stream.id).nil?)
          is_instance_streams = true
          break
        end
      end
    end

    if(is_instance_streams)
      flash[:error] = "There are analytic instances that are using video streams from this camera. Unable to delete the camera -  #{@vms.name}; "
    else
      @vms.destroy
      flash[:notice] = 'The camera details was successfully deleted.'
    end

    redirect_to direct_cameras_url
  end

  private
  # Use callbacks to share common setup or constraints between actions.
  def set_vms
    @vms = Vms.find(params[:id])
  end

  def set_vms_connector
    @vms_connector = Vms.find(params[:id]).vms_connector
  end

  # Never trust parameters from the scary internet, only allow the white list through.
  def vms_params
    params.require(:vms).permit(:name, :description, :vms_connector_id, :camera_url)
  end

end
