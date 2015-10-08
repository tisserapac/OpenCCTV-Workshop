class RemoveTypeFromVmsConnectors < ActiveRecord::Migration
  def change
    remove_column :vms_connectors, :type, :string
  end
end
