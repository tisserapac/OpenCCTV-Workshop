class AnalyticInstancesController < ApplicationController
  before_action :set_analytic_instance, only: [:show, :edit, :update, :destroy, :start_analytic, :stop_analytic]
  respond_to :html

  # GET /analytic_instances
  def index
    @analytic_instances = AnalyticInstance.all
    respond_with(@analytic_instances)
  end

  # GET /analytic_instances/1
  def show
    respond_with(@analytic_instance)
  end

  # GET /analytic_instances/new
  def new
    @analytic_instance = AnalyticInstance.new
    respond_with(@analytic_instance)
  end

  # GET /analytic_instances/1/edit
  def edit
  end

  # POST /analytic_instances
  def create
    @analytic_instance = AnalyticInstance.new(analytic_instance_params)
    @analytic_instance.status = 'Stopped'
    @analytic_instance.save
    respond_with(@analytic_instance)
  end

  # PATCH/PUT /analytic_instances/1
  def update
    if(@analytic_instance.status.eql? 'Stopped')
      @analytic_instance.update(analytic_instance_params)
    else
      flash[:error] = "Unable to update the analytic instance #{@analytic_instance.id}; Stop the analytic instance first, in order to edit its details"
    end

    respond_with(@analytic_instance)
  end

  # DELETE /analytic_instances/1
  def destroy
    if(@analytic_instance.status.eql? 'Stopped')
      @analytic_instance.destroy
    else
      flash[:error] = "Unable to delete the analytic instance #{@analytic_instance.id}; Stop the analytic instance first, in order to delete it"
    end
    respond_with(@analytic_instance)
  end

  def start_analytic
    if(@analytic_instance.analytic_instance_streams.empty?)
      flash[:error] = "Unable to start analytic instance #{@analytic_instance.id}; no inputs streams are configured with this analytic instance"
    else
      server_reply = @analytic_instance.exec_analytic_event('Analytic Start')
      if(server_reply[:type].eql? 'Error')
        flash[:error] = "#{server_reply[:content]}"
      else
        flash[:notice] = "#{server_reply[:content]}"
      end
    end
    #redirect_to analytic_instances_path
    redirect_to :back
  end

  def stop_analytic
    server_reply = @analytic_instance.exec_analytic_event('Analytic Stop')
    if(server_reply[:type].eql? 'Error')
      flash[:error] = "#{server_reply[:content]}"
    else
      flash[:notice] = "#{server_reply[:content]}"
    end
    #redirect_to analytic_instances_path
    redirect_to :back
  end

  private
    # Use callbacks to share common setup or constraints between actions.
    def set_analytic_instance
      @analytic_instance = AnalyticInstance.find(params[:id])
    end

    # Never trust parameters from the scary internet, only allow the white list through.
    def analytic_instance_params
      params.require(:analytic_instance).permit(:name, :description, :analytic_id)
    end
end
