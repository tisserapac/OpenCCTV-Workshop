class AddCameraUrlToVmses < ActiveRecord::Migration
  def change
    add_column :vmses, :camera_url, :string
  end
end
