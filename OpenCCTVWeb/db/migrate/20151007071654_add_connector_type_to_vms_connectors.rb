class AddConnectorTypeToVmsConnectors < ActiveRecord::Migration
  def change
    add_column :vms_connectors, :connector_type, :string
  end
end
