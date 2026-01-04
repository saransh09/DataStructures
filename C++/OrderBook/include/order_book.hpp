#include <algorithm>
#include <cstddef>
#include <unordered_set>
#include <vector>

namespace ds {

using Id = size_t;
using Price = long;
using Quantity = int;

class Order {
public:
  Order(Id orderId, Price level, bool isBuy, Quantity quantity)
      : order_id_(orderId), level_(level), is_buy_(isBuy), quantity_(quantity) {
  }

  Id get_order_id() const { return order_id_; }

  Id OrderId() const { return order_id_; }

  bool is_buy_order() const { return is_buy_; }

  Price get_level() const { return level_; }

  Quantity get_quantity() const { return quantity_; }

  void decrease_quantity(Quantity q) {
    if (q > quantity_)
      return;
    quantity_ -= q;
  }

private:
  Id order_id_{};
  Price level_{};
  bool is_buy_{};
  Quantity quantity_{};
};

using Orders = std::vector<Order>;

// DO NOT MODIFY.
struct Trade {
  Id OrderIdA;
  Id OrderIdB; // Aggressor's OrderId
  Id AggressorOrderId;
  bool AggressorIsBuy;
  Price Level;
  Quantity Size;
};

using Trades = std::vector<Trade>;

inline bool sort_bids(const Order &bid1, const Order &bid2) {
  return bid1.get_level() > bid2.get_level();
}

inline bool sort_asks(const Order &ask1, const Order &ask2) {
  return ask1.get_level() < ask2.get_level();
}

class Orderbook {
public:
  // Implement AddOrder and CancelOrder.
  Trades AddOrder(const Order &order) {
    // check if this order already exists in the order book
    if (existing_order_ids_.contains(order.get_order_id())) {
      return {};
    }
    // add the order_id of the order
    existing_order_ids_.insert(order.get_order_id());
    // add the order in the appropriate list
    if (order.is_buy_order()) {
      bids_.emplace_back(order);
      std::stable_sort(bids_.begin(), bids_.end(), sort_bids);
    } else {
      asks_.emplace_back(order);
      std::stable_sort(asks_.begin(), asks_.end(), sort_asks);
    }
    // execute the order
    return ExecuteTrades(order.get_order_id());
  }

  void CancelOrder(Id orderId) {
    if (!existing_order_ids_.contains(orderId)) {
      return;
    }
    auto remove = [&](Orders &orders) {
      for (auto it = orders.begin(); it != orders.end(); ++it) {
        if (it->get_order_id() == orderId) {
          orders.erase(it);
          existing_order_ids_.erase(orderId);
          return true;
        }
      }
      return false;
    };
    if (!remove(bids_)) {
      remove(asks_);
    }
  }

  Trades ExecuteTrades(Id aggressor_id) {
    Trades trades;

    while (!bids_.empty() && !asks_.empty()) {
      Order &bid = bids_.front();
      Order &ask = asks_.front();

      if (bid.get_level() < ask.get_level()) {
        break;
      }

      Quantity traded_qty = std::min(bid.get_quantity(), ask.get_quantity());

      bool aggressor_is_buy = (aggressor_id == bid.get_order_id());

      trades.push_back(
          Trade{.OrderIdA = bid.get_order_id(),
                .OrderIdB = ask.get_order_id(),
                .AggressorOrderId = aggressor_id,
                .AggressorIsBuy = aggressor_is_buy,
                .Level = aggressor_is_buy ? bid.get_level() : ask.get_level(),
                .Size = traded_qty});

      bid.decrease_quantity(traded_qty);
      ask.decrease_quantity(traded_qty);

      if (bid.get_quantity() == 0) {
        existing_order_ids_.erase(bid.get_order_id());
        bids_.erase(bids_.begin());
      }

      if (ask.get_quantity() == 0) {
        existing_order_ids_.erase(ask.get_order_id());
        asks_.erase(asks_.begin());
      }
    }
    return trades;
  }

private:
  Orders bids_;
  Orders asks_;
  std::unordered_set<Id> existing_order_ids_;
};
} // namespace ds
