#pragma once
#include <string>
#include <stdexcept>
#include "lab2_files/Sequence.hpp"
#include "lab2_files/ArraySequence.hpp"
#include "MyUtils.hpp"

namespace fsm {
    
struct State {
    std::string id;
    bool is_final;
    State() : id( "" ), is_final( false ) { }
    State( const std::string& state_id, bool final_state = false ) : id( state_id ), is_final( final_state ) { }
};

template <typename T>
class Transition {
public:
    typedef bool ( *PredicateFunc )( const T& );

    Transition() : from_state( "" ), to_state( "" ), condition_( nullptr ) {}
    Transition( const std::string& from, const std::string& to, PredicateFunc condition )
        : from_state( from ), to_state( to ), condition_( condition ) { }

    bool CanTransition( const std::string& current_state, const T& input ) const {
        return ( current_state == from_state ) && condition_( input );
    }
    std::string GetNextState() const { return to_state; }

private:
    std::string  from_state;
    std::string  to_state;
    PredicateFunc condition_;
};

template <typename T>
class StateMachine {
public:
    StateMachine() : current_state_id_( "" ) {}

    void AddState( const std::string& id, bool is_final = false ) {
        if ( FindState( id ) != -1 ) throw std::invalid_argument( "StateMachine: State exists" );
        states_.Append( State( id, is_final ) );
    }

    void SetInitialState( const std::string& id ) {
        if ( FindState( id ) == -1 ) throw std::invalid_argument( "StateMachine: State not found" );
        initial_state_id_ = id;
        current_state_id_ = id;
    }

    void AddTransition( const std::string& from, const std::string& to, typename Transition<T>::PredicateFunc condition ) {
        if (FindState( from ) == -1 || FindState( to ) == -1 )
            throw std::invalid_argument( "StateMachine: invalid states" );
        transitions_.Append( Transition<T>( from, to, condition ) );
    }

    bool Process( const Sequence<T>* input_stream ) {
        Reset();
        IEnumerator<T>* enumerator = input_stream->GetEnumerator();

        while ( enumerator->MoveNext() ) {
            const T& current_input = enumerator->Current();
            bool transitioned = false;

            for ( int i = 0; i < transitions_.GetLength(); ++i ) {
                const Transition<T>& transition = transitions_.Get( i );
                if ( transition.CanTransition( current_state_id_, current_input ) ) {
                    current_state_id_ = transition.GetNextState();
                    transitioned = true;
                    break; 
                }
            }
            if ( !transitioned ) { delete enumerator; return false; }
        }
        delete enumerator;
        return IsInFinalState();
    }

    void Reset() { current_state_id_ = initial_state_id_; }
    std::string GetCurrentState() const { return current_state_id_; }

private:
    MutableArraySequence<State>         states_;
    MutableArraySequence<Transition<T>> transitions_;
    std::string                         initial_state_id_;
    std::string                         current_state_id_;

    int FindState( const std::string& id ) const {
        for ( int i = 0; i < states_.GetLength(); ++i ) 
            if ( states_.Get( i ).id == id ) return i;
        return -1;
    }

    bool IsInFinalState() const {
        int idx = FindState( current_state_id_ );
        return ( idx != -1 ) ? states_.Get( idx ).is_final : false;
    }
};

}
