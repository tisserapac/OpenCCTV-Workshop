class DirectCamerasController < ApplicationController
  def index
    @vmses = Vms.joins(:vms_connector).where('vms_connectors.connector_type = "direct_ip_camera"')
  end

  def show
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
    #TODO Carry out actual validation of the direct camera
    @vms.verified = true
    #TODO Assign a suitable VMS type
    @vms.vms_type = 'NA'

    @vms.save

=begin
    if(!@vms.errors.any?)
      begin
        if(@vms.validate_connection)
          flash[:notice] = 'Successfully connected to the VMS.'
          @vms.add_cameras
        end
      rescue Exception => e
        flash[:error] = e.message
      end
      respond_with @vms
    end
=end

    redirect_to direct_cameras_url
  end

  def update
  end

  def destroy
  end

  private
  # Never trust parameters from the scary internet, only allow the white list through.
  def vms_params
    params.require(:vms).permit(:name, :description, :vms_connector_id, :camera_url)
  end

end
