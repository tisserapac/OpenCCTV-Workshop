class AddTypeToVmsConnectors < ActiveRecord::Migration
  def change
    add_column :vms_connectors, :type, :string
  end
end
