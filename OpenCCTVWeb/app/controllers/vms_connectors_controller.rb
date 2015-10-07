class VmsConnectorsController < ApplicationController
  before_action :set_vms_connector, only: [:show, :edit, :update, :destroy]
  respond_to :html

  def new
    @vms_connector = VmsConnector.new
    respond_with @vms_connector
  end

  def create
    @vms_connector = VmsConnector.new(vms_connector_params)
    #@vms_connector.type = params[:vms_connector][:type]
    uploaded_io = params[:vms_connector][:file]
    if(uploaded_io && uploaded_io.content_type.to_s == 'application/zip')
      @vms_connector.filename = SecureRandom.uuid.to_s
      File.open(Rails.root.join('app/uploads', 'vms_connectors', (@vms_connector.filename  + '.zip')), 'wb') do |f|
        f.write(uploaded_io.read)
      end
      if(@vms_connector.validate_plugin_archive())
        flash[:notice] = 'Uploaded the new VMS connector and verified.'
      else
        flash[:error] = 'Failed to verify the uploaded VMS connector.'
      end
      @vms_connector.save
    else
      flash[:error] = 'Failed to upload the new VMS connector. No VMS connector plugin archive (.zip) was selected or invalid file type.'
    end
    respond_with @vms_connector
  end

  def index
    @vms_connectors = VmsConnector.all
    respond_with @vms_connectors
  end

  def show
    respond_with @vms_connector
  end

  def edit
  end

  def update
    @vms_connector.update(vms_connector_params)
    respond_with @vms_connector
  end

  def destroy
    is_instance_streams = false

    @vms_connector.vmses.each do |vms|
      vms.cameras.each do |camera|
        camera.streams.each do |stream|
          if(!AnalyticInstanceStream.find_by_stream_id(stream.id).nil?)
            is_instance_streams = true
            break
          end
        end
      end
    end

    if(is_instance_streams)
      flash[:error] = "There are analytic instances that are using VMSs conected via this connector type . Unable to delete the VMS connector plugin -  #{@vms_connector.name}; "
    else
      path_to_file = Rails.root.join('app/uploads', 'vms_connectors', (@vms_connector.filename + '.zip'))
      File.delete(path_to_file) if File.exist?(path_to_file)
      @vms_connector.destroy
    end
    #respond_with @vms_connector
    redirect_to vms_connectors_path
  end

  private
  def set_vms_connector
    @vms_connector = VmsConnector.find(params[:id])
  end

  def vms_connector_params
    params.require(:vms_connector).permit(:name, :description, :connector_type)
  end

end
