class DownloadController < ApplicationController
  def vms_connector
    path_to_file = Rails.root.join('app/uploads', 'vms_connectors', (params[:filename].to_s + '.zip'))
    if (File.exist?(path_to_file))
      send_file(path_to_file.to_s, :type => "application/zip", :x_sendfile => true, :status => :ok)
    else
      flash[:error] = "Unable to locate the #{params[:filename]}.zip file"
      redirect_to vms_connectors_path
    end
  end

  def analytic
    path_to_file = Rails.root.join('app/uploads', 'analytics', (params[:filename].to_s + '.zip'))
    if (File.exist?(path_to_file))
      send_file(path_to_file.to_s, :type => "application/zip", :x_sendfile => true, :status => :ok)
    else
      flash[:error] = "Unable to locate the #{params[:filename]}.zip file"
      redirect_to analytics_path
    end
  end
end
