class StreamsController < ApplicationController
  before_action :set_stream, only: [:show, :edit, :update, :destroy]
  before_action :set_camera, only: [:index, :new, :create, :show, :edit, :destroy]
  before_action :set_vms, only: [:new, :show, :edit, :destroy]
  before_action :set_vms_connector, only: [:show]
  respond_to :html

  def index
    redirect_to vms_camera_path(@camera.vms, @camera)
  end

  def show
  end

  def new
    @stream = Stream.new
    @stream.camera = @camera
    respond_with(@stream)
  end

  def edit
  end

  def create
    @stream = @camera.streams.create(:name => params[:stream][:name], :width => params[:stream][:width], :height => params[:stream][:height], :keep_aspect_ratio => params[:stream][:keep_aspect_ratio], :allow_upsizing => params[:stream][:allow_upsizing], :compression_rate => params[:stream][:compression_rate], :description => params[:stream][:description])
    if (!@stream.errors.any?)
      if (@stream.camera.vms.vms_connector.name == "Milestone")
        if(@stream.milestone_validate_stream)
          @stream.set_verification(true)
          is_stream_valid, width, height = @stream.milestone_grab_stream_frame
        else
          @stream.set_verification(false)
        end
      else
      end
      #redirect_to vms_camera_stream_path(@stream.camera.vms, @stream.camera, @stream)
    end

    redirect_to vms_camera_path(@stream.camera.vms, @stream.camera)

  end

  def update
    if(@stream.analytic_instance_streams.empty?)
      @stream.update(:name => params[:stream][:name], :width => params[:stream][:width], :height => params[:stream][:height], :keep_aspect_ratio => params[:stream][:keep_aspect_ratio], :allow_upsizing => params[:stream][:allow_upsizing], :compression_rate => params[:stream][:compression_rate], :description => params[:stream][:description])
      if (!@stream.errors.any?)
        if (@stream.camera.vms.vms_connector.name == "Milestone")
          if(@stream.milestone_validate_stream)
            @stream.set_verification(true)
            is_stream_valid, width, height = @stream.milestone_grab_stream_frame
          else
            @stream.set_verification(false)
          end
        else
          #Validation of other types of VMS connectors
        end
      end
    else
      flash[:error] = "There are analytic instances that are using this video stream. Unable to edit the stream #{@stream.name}; "
    end

    redirect_to vms_camera_path(@stream.camera.vms, @stream.camera)

    #redirect_to vms_camera_stream_path(@stream.camera.vms, @stream.camera, @stream)

  end

  def destroy
    if(@stream.analytic_instance_streams.empty?)
      @stream.destroy
    else
      flash[:error] = "There are analytic instances that are using this video stream. Unable to delete the stream #{@stream.name}; "
    end
    redirect_to vms_camera_path(@vms, @camera)
  end
end

private
# Use callbacks to share common setup or constraints between actions.
def set_stream
  @stream = Stream.find(params[:id])
end

def set_camera
  @camera = Camera.find(params[:camera_id])
end

def set_vms
  @vms = Vms.find(params[:vms_id])
end

def set_vms_connector
  @vms_connector = Vms.find(params[:vms_id]).vms_connector
end

# Never trust parameters from the scary internet, only allow the white list through.
def stream_params
  params.require(:stream).permit(:name, :description, :width, :height, :keep_aspect_ratio, :allow_upsizing, :compression_rate, :verified, :camera_id)
end