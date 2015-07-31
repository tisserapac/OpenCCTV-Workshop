class AddStatusToAnalyticInstances < ActiveRecord::Migration
  def change
    add_column :analytic_instances, :status, :string
  end
end
